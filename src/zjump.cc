/**
    Copyright (c) 2017 Vicente Romero. All rights reserved.
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.
*/

#include <cstdio>
#include <cstring>
#include <string>

#include "compress.h"
#include "constants.h"
#include "decompress.h"

using namespace std;

struct ExecConfig {
    bool help_opt;
    bool decompress_opt;
    bool stdout_opt;
    bool force_opt;
    bool version_opt;
    string in_file_name;
    string out_file_name;
    FILE *in_file;
    FILE *out_file;

    ExecConfig() {
        help_opt        = false;
        decompress_opt  = false;
        stdout_opt      = false;
        force_opt       = false;
        version_opt     = false;
        in_file         = stdin;
        out_file        = stdout;
    }

    ~ExecConfig() {
        if((in_file != nullptr) && (in_file != stdin)) {
            fclose(in_file);
        }

        if((out_file != nullptr) && (out_file != stdout)) {
            fclose(out_file);
        }
    }
};

static const char *kZjumpCompressedExt      = ".zjump";
static const char *kZjumpDecompressedExt    = ".orig";

static void DisplayVersionNumber() {
    uint32_t major = kZjumpVersion / 10000;
    uint32_t minor = (kZjumpVersion / 100) % 100;
    uint32_t patch = kZjumpVersion % 100;
    fprintf(stderr, "zjump %u.%u.%u\n", major, minor, patch);
}

static void Usage(const char *name)
{
    fprintf(stderr,
"zjump, a file compressor/decompressor\n"
"\n"
"Usage: %s [OPTIONS] [FILE]\n"
"\n"
"  -c, --stdout         Write on standard output\n"
"  -d, --decompress     Decompress FILE\n"
"  -f, --force          Force to overwrite the output file\n"
"  -h, --help           Output this help and exit\n"
"  -V, --version        Display version number\n"
"\n"
"If no FILE is given, zjump compresses or decompresses\n"
"from standard input to standard output."
"\n",
    name);
}

static void SetOutputFileName(ExecConfig* config) {
    if(config->stdout_opt || (config->in_file_name.empty())) {
        return;
    }

    if(config->decompress_opt) {
        size_t last_dot = config->in_file_name.find_last_of('.');

        if( (last_dot == string::npos) ||
            (config->in_file_name.compare(last_dot, string::npos, kZjumpCompressedExt) != 0)) {
            config->out_file_name = config->in_file_name + kZjumpDecompressedExt;
        } else {
            config->out_file_name = config->in_file_name.substr(0, last_dot);
        }
    } else {
        config->out_file_name = config->in_file_name + kZjumpCompressedExt;
    }
}

static int ParseOptions(int argc, char **argv, ExecConfig* config) {
    for(int i=1; i<argc; ++i) {
        if((strcmp(argv[i], "-c") == 0) || (strcmp(argv[i], "--stdout") == 0)) {
            config->stdout_opt = true;
        } else if((strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "--decompress") == 0)) {
            config->decompress_opt = true;
        } else if((strcmp(argv[i], "-f") == 0) || (strcmp(argv[i], "--force") == 0)) {
            config->force_opt = true;
        } else if((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)) {
            config->help_opt = true;
        } else if((strcmp(argv[i], "-V") == 0) || (strcmp(argv[i], "--version") == 0)) {
            config->version_opt = true;
        } else if(argv[i][0] == '-') {
            fprintf(stderr, "Unrecognized option: '%s'\n", argv[i]);
        } else {
            return i;
        }
    }

    return argc;
}

static ZjumpErrorCode ValidateOptions(int argc, char **argv, int last_opt, ExecConfig* config) {
    if(config->help_opt) {
        return ZJUMP_NO_ERROR;
    }

    int last_args = argc - last_opt;
    if(last_args > 1) {
        fprintf(stderr, "Incorrect arguments. Use -h to display more information\n");
        return ZJUMP_ERROR_ARGUMENT;
    } else if(last_args == 1) {
        config->in_file_name = argv[last_opt];
        SetOutputFileName(config);
    }

    return ZJUMP_NO_ERROR;
}

static bool FileExists(const char* file_name) {
    FILE *file = fopen(file_name, "rb");
    if(file == nullptr) {
        return false;
    } else {
        fclose(file);
        return true;
    }
}

static ZjumpErrorCode ValidateOutput(const ExecConfig& config) {
    if(config.force_opt) {
        return ZJUMP_NO_ERROR;
    }

    if(config.out_file_name.empty()) {
        return ZJUMP_NO_ERROR;
    }

    const char *out_file_name = config.out_file_name.c_str();

    if(FileExists(out_file_name)) {
        fprintf(stderr, "Output file %s already exists.\n", out_file_name);
        return ZJUMP_ERROR_FILE;
    }

    return ZJUMP_NO_ERROR;
}

static ZjumpErrorCode OpenFiles(ExecConfig* config) {
    if(!config->in_file_name.empty()) {
        const char *in_file_name = config->in_file_name.c_str();

        config->in_file = fopen(in_file_name, "rb");

        if(config->in_file == nullptr) {
            perror(in_file_name);
            return ZJUMP_ERROR_FILE;
        }
    }

    if(!config->out_file_name.empty()) {
        const char *out_file_name = config->out_file_name.c_str();

        config->out_file = fopen(out_file_name, "wb");

        if(config->out_file == nullptr) {
            perror(out_file_name);
            return ZJUMP_ERROR_FILE;
        }
    }

    return ZJUMP_NO_ERROR;
}

int main(int argc, char **argv) {
    ExecConfig config;

    int last_opt = ParseOptions(argc, argv, &config);

    int ret_code = ValidateOptions(argc, argv, last_opt, &config);
    if(ret_code != ZJUMP_NO_ERROR) {
        return ret_code;
    }

    if(config.version_opt) {
        DisplayVersionNumber();
        return ZJUMP_NO_ERROR;
    }

    if(config.help_opt) {
        Usage(argv[0]);
        return ZJUMP_NO_ERROR;
    }

    ret_code = ValidateOutput(config);
    if(ret_code != ZJUMP_NO_ERROR) {
        return ret_code;
    }

    ret_code = OpenFiles(&config);
    if(ret_code != ZJUMP_NO_ERROR) {
        return ret_code;
    }

    if(config.decompress_opt) {
        Decompressor decompressor;
        ret_code = decompressor.Decompress(config.in_file, config.out_file);
        if(ret_code != ZJUMP_NO_ERROR) {
            return ret_code;
        }
    } else {
        Compressor compressor;
        ret_code = compressor.Compress(config.in_file, config.out_file);
        if(ret_code != ZJUMP_NO_ERROR) {
            return ret_code;
        }
    }

    return ZJUMP_NO_ERROR;
}

