"""
Blender batch FBX -> GLB converter.

Usage:
  blender -b -P scripts/fbx_to_glb.py -- --model path/to/model.fbx --anim path/to/idle.fbx --anim ... --out out.glb

Notes:
- Imports a rigged model FBX first (mesh + armature).
- Imports each animation FBX, collects any newly created Actions, then deletes the imported objects.
- Exports a single GLB containing the original mesh/armature + all imported Actions.
"""

import argparse
import os
import sys


def _parse_args(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--model", required=True)
    p.add_argument("--anim", action="append", default=[])
    p.add_argument("--out", required=True)
    return p.parse_args(argv)


def _blender_main(argv):
    import bpy  # noqa: F401

    # Clean scene.
    bpy.ops.wm.read_factory_settings(use_empty=True)

    args = _parse_args(argv)

    model_path = os.path.abspath(args.model)
    anim_paths = [os.path.abspath(a) for a in args.anim]
    out_path = os.path.abspath(args.out)

    if not os.path.exists(model_path):
        raise SystemExit(f"Missing model: {model_path}")
    for a in anim_paths:
        if not os.path.exists(a):
            raise SystemExit(f"Missing anim: {a}")

    # Import base rig/mesh.
    bpy.ops.import_scene.fbx(filepath=model_path, use_anim=True)

    # Pick the main armature as the first imported armature object.
    armatures = [o for o in bpy.data.objects if o.type == "ARMATURE"]
    if not armatures:
        raise SystemExit("No ARMATURE found after importing model FBX.")
    main_arm = armatures[0]

    # Ensure actions exist list.
    existing_actions = set(bpy.data.actions)

    # Import animations; keep any new Actions.
    for ap in anim_paths:
        before_actions = set(bpy.data.actions)
        before_objs = set(bpy.data.objects)

        bpy.ops.import_scene.fbx(filepath=ap, use_anim=True)

        after_actions = set(bpy.data.actions)
        new_actions = [a for a in after_actions - before_actions if a is not None]

        # Rename actions based on filename for friendliness.
        base = os.path.splitext(os.path.basename(ap))[0]
        for a in new_actions:
            # Avoid collisions.
            if not a.name.lower().startswith(base.lower()):
                a.name = f"{base}_{a.name}"

        # Delete newly imported objects (armature/mesh duplicates), but keep the Actions datablocks.
        after_objs = set(bpy.data.objects)
        new_objs = list(after_objs - before_objs)
        if new_objs:
            bpy.ops.object.select_all(action="DESELECT")
            for o in new_objs:
                o.select_set(True)
            bpy.ops.object.delete(use_global=False)

        # Try to ensure actions are compatible: bone names must match. We can't validate fully here.
        existing_actions |= set(new_actions)

    # Ensure the main armature has at least one action bound for export; glTF exporter will export all actions.
    if main_arm.animation_data is None:
        main_arm.animation_data_create()
    # Bind an arbitrary action if present so some exporters keep them; glTF exporter should still export all.
    any_action = next(iter(bpy.data.actions), None)
    if any_action is not None and main_arm.animation_data.action is None:
        main_arm.animation_data.action = any_action

    os.makedirs(os.path.dirname(out_path) or ".", exist_ok=True)

    # Export to GLB.
    bpy.ops.export_scene.gltf(
        filepath=out_path,
        export_format="GLB",
        export_yup=True,
        export_apply=True,
        export_animations=True,
        export_skins=True,
        export_morph=False,
        export_materials="EXPORT",
        export_colors=True,
        export_normals=True,
        export_texcoords=True,
        export_tangents=False,
        export_image_format="AUTO",
    )

    print(f"Wrote {out_path}")


def main():
    # Blender passes args after `--` to the script.
    if "--" not in sys.argv:
        raise SystemExit("Expected Blender args separator `--`.")
    idx = sys.argv.index("--")
    _blender_main(sys.argv[idx + 1 :])


if __name__ == "__main__":
    main()

