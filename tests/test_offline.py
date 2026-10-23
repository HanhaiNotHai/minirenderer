"""Exercise the offline executable and inspect the actual P6 image bytes."""

import pathlib
import subprocess
import sys
import tempfile
import unittest


EXECUTABLE = pathlib.Path(sys.argv.pop(1)).resolve()


class OfflineTests(unittest.TestCase):
    def check_image(self, path):
        data = path.read_bytes()
        header = b"P6\n64 64\n255\n"
        self.assertTrue(data.startswith(header))
        pixels = data[len(header):]
        self.assertEqual(len(pixels), 12288)
        white_points = {(7, 3), (12, 37), (62, 53)}
        for y in range(64):
            for x in range(64):
                offset = (y * 64 + x) * 3
                expected = b"\xff\xff\xff" if (x, y) in white_points else b"\0\0\0"
                self.assertEqual(pixels[offset:offset + 3], expected, (x, y))

    def test_default_output_has_exactly_three_white_points(self):
        with tempfile.TemporaryDirectory() as directory:
            result = subprocess.run([EXECUTABLE], cwd=directory, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.check_image(pathlib.Path(directory) / "framebuffer.ppm")

    def test_explicit_output_path_with_spaces(self):
        with tempfile.TemporaryDirectory() as directory:
            output = pathlib.Path(directory) / "three points.ppm"
            result = subprocess.run([EXECUTABLE, output], capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.check_image(output)

    def test_missing_parent_directory_is_an_error(self):
        with tempfile.TemporaryDirectory() as directory:
            output = pathlib.Path(directory) / "missing" / "frame.ppm"
            result = subprocess.run([EXECUTABLE, output], capture_output=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertTrue(result.stderr)

    def test_extra_arguments_are_an_error(self):
        result = subprocess.run([EXECUTABLE, "one.ppm", "two.ppm"], capture_output=True)
        self.assertNotEqual(result.returncode, 0)


if __name__ == "__main__":
    unittest.main()
