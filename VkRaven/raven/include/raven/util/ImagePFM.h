#pragma once
// https://github.com/dscharstein/pfmLib

#include <fstream>
#include <iomanip>
#include <iostream>

namespace raven {
    class ImagePFM {
    public:
        enum ImagePFMType {
            GRAYSCALE = 0,
            COLOR = 1
        };

        // check whether machine is little endian
        static int littleendian() {
            int intval = 1;
            const auto uval = reinterpret_cast<unsigned char *>(&intval);
            return uval[0] == 1;
        }

        static void swapBytes(float *fptr) {
            // if endianness doesn't agree, swap bytes
            const auto ptr = reinterpret_cast<unsigned char *>(fptr);
            unsigned char tmp = ptr[0];
            ptr[0] = ptr[3];
            ptr[3] = tmp;
            tmp = ptr[1];
            ptr[1] = ptr[2];
            ptr[2] = tmp;
        }

        /*
         *  Reads a .pfm file image file into an
         *  opencv Mat structure with type
         *  CV_32F, handles either 1 band or 3 band
         *  images
         *
         *  Params:
         *      im:     type: Mat       description: image destination
         *      path:   type: string    description: file path to pfm file
         */
        static int readFilePFM(std::vector<float> &im, const std::string &path) {

            // create fstream object to read in pfm file
            // open the file in binary
            std::fstream file(path.c_str(), std::ios::in | std::ios::binary);

            // init variables
            std::string bands; // what type is the image   "Pf" = grayscale    (1-band)
            //                          "PF" = color        (3-band)
            int width, height; // width and height of the image
            float scalef; // scale factor
            float fvalue0; // temp value to hold pixel value
            float fvalue1;
            float fvalue2;


            // extract header information, skips whitespace
            file >> bands;
            file >> width;
            file >> height;
            file >> scalef;

            // determine endianness
            int littleEndianFile = (scalef < 0);
            int littleEndianMachine = littleendian();
            int needSwap = (littleEndianFile != littleEndianMachine);

            std::cout << std::setfill('=') << std::setw(19) << "=" << std::endl;
            std::cout << "Reading image to pfm file: " << path << std::endl;
            std::cout << "Little Endian?: " << ((needSwap) ? "false" : "true") << std::endl;
            std::cout << "width: " << width << std::endl;
            std::cout << "height: " << height << std::endl;
            std::cout << "scale: " << scalef << std::endl;

            // skip SINGLE newline character after reading third arg
            char c = static_cast<char>(file.get());
            if (c == '\r') // <cr> in some files before newline
                c = static_cast<char>(file.get());
            if (c != '\n') {
                if (c == ' ' || c == '\t' || c == '\r') {
                    std::cout << "newline expected";
                    return -1;
                } else {
                    std::cout << "whitespace expected";
                    return -1;
                }
            }

            if (bands == "Pf") {
                // handle 1-band image
                std::cout << "Reading grayscale image (1-band)" << std::endl;
                im.resize(width * height, 0); // im = Mat::zeros(height, width, CV_32FC1);
                for (int i = height - 1; i >= 0; --i) {
                    for (int j = 0; j < width; ++j) {
                        file.read(reinterpret_cast<char *>(&fvalue0), sizeof(fvalue0));
                        if (needSwap) {
                            swapBytes(&fvalue0);
                        }
                        im[i * width + j] = fvalue0; // im.at<float>(i, j) = (float) fvalue;
                    }
                }
            } else if (bands == "PF") {
                // handle 3-band image
                std::cout << "Reading color image (3-band)" << std::endl;
                im.resize(3 * width * height, 0); // im = Mat::zeros(height, width, CV_32FC3);
                for (int i = height - 1; i >= 0; --i) {
                    for (int j = 0; j < width; ++j) {
                        file.read(reinterpret_cast<char *>(&fvalue0), sizeof(fvalue0));
                        file.read(reinterpret_cast<char *>(&fvalue1), sizeof(fvalue1));
                        file.read(reinterpret_cast<char *>(&fvalue2), sizeof(fvalue2));
                        if (needSwap) {
                            swapBytes(&fvalue0);
                            swapBytes(&fvalue1);
                            swapBytes(&fvalue2);
                        }
                        im[4 * (i * width + j) + 0] = fvalue0; // im.at<Vec3f>(i, j) = vfvalue;
                        im[4 * (i * width + j) + 1] = fvalue1; // im.at<Vec3f>(i, j) = vfvalue;
                        im[4 * (i * width + j) + 2] = fvalue2; // im.at<Vec3f>(i, j) = vfvalue;
                        im[4 * (i * width + j) + 3] = 1;
                    }
                }
            } else {
                std::cout << "unknown bands description";
                return -1;
            }
            std::cout << std::setfill('=') << std::setw(19) << "=" << std::endl << std::endl;
            return 0;
        }

        /*
         *  Writes a .pfm file image file from an
         *  opencv Mat structure with type
         *  CV_32F, handles either 1 band or 3 band
         *  images
         *
         *  Params:
         *      im:     type: Mat       description: image destination
         *      path:   type: string    description: file path to pfm file
         *      scalef: type: float     description: scale factor and endianness
         */
        static int writeFilePFM(const std::vector<float> &im, const ImagePFMType &type, const int width, const int height, const std::string &path, float scalef = 1) {

            // create fstream object to write out pfm file
            // open the file in binary
            std::fstream file(path.c_str(), std::ios::out | std::ios::binary);


            // init variables
            std::string bands;
            float fvalue0; // scale factor and temp value to hold pixel value
            float fvalue1;
            float fvalue2;


            switch (type) {
                // determine identifier string based on image type
                case GRAYSCALE:
                    bands = "Pf"; // grayscale
                    break;
                case COLOR:
                    bands = "PF"; // color
                    break;
                default:
                    std::cout << "Unsupported image type, must be GRAYSCALE or COLOR";
                    return -1;
            }

            // sign of scalefact indicates endianness, see pfms specs
            if (littleendian())
                scalef = -scalef;

            // insert header information
            file << bands << "\n";
            file << width << "\n";
            file << height << "\n";
            file << scalef << "\n";

            std::cout << std::setfill('=') << std::setw(19) << "=" << std::endl;
            std::cout << "Writing image to pfm file: " << path << std::endl;
            std::cout << "Little Endian?: " << ((littleendian()) ? "true" : "false") << std::endl;
            std::cout << "width: " << width << std::endl;
            std::cout << "height: " << height << std::endl;
            std::cout << "scale: " << scalef << std::endl;

            if (bands == "Pf") {
                // handle 1-band image
                std::cout << "Writing grayscale image (1-band)" << std::endl;
                for (int i = height - 1; i >= 0; --i) {
                    for (int j = 0; j < width; ++j) {
                        fvalue0 = im[i * width + j]; // im.at<float>(i, j);
                        file.write(reinterpret_cast<char *>(&fvalue0), sizeof(fvalue0));

                    }
                }
            } else if (bands == "PF") {
                // handle 3-band image
                std::cout << "writing color image (3-band)" << std::endl;
                for (int i = height - 1; i >= 0; --i) {
                    for (int j = 0; j < width; ++j) {
                        fvalue0 = im[4 * (i * width + j) + 0]; // im.at<glm::vec3>(i, j);
                        fvalue1 = im[4 * (i * width + j) + 1];
                        fvalue2 = im[4 * (i * width + j) + 2];
                        file.write(reinterpret_cast<char *>(&fvalue0), sizeof(fvalue0));
                        file.write(reinterpret_cast<char *>(&fvalue1), sizeof(fvalue1));
                        file.write(reinterpret_cast<char *>(&fvalue2), sizeof(fvalue2));
                    }
                }
            } else {
                std::cout << "unknown bands description";
                return -1;
            }
            std::cout << std::setfill('=') << std::setw(19) << "=" << std::endl << std::endl;
            return 0;
        }
    };
}