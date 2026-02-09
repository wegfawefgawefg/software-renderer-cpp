import argparse


def process_obj_file(input_filename, output_filename):
    try:
        with open(input_filename, "r") as infile, open(output_filename, "w") as outfile:
            # Count faces by identifying lines starting with 'f '
            face_count = sum(1 for line in infile if line.startswith("f "))

            # Write "255 255 255 255" for each face found
            for _ in range(face_count):
                outfile.write("255 255 255 255\n")

        print(f"Processed {face_count} faces. Output written to {output_filename}.")

    except FileNotFoundError:
        print(f"The file {input_filename} does not exist.")
    except Exception as e:
        print(f"An error occurred: {e}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Process OBJ file to generate an output file based on face count."
    )
    parser.add_argument("input_file", help="Path to the input OBJ file")
    parser.add_argument("output_file", help="Path to the output file")

    args = parser.parse_args()

    process_obj_file(args.input_file, args.output_file)
