/* **********************************************************
 * Copyright (c) 2016-2018 Google, Inc.  All rights reserved.
 * **********************************************************/

/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Google, Inc. nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL VMWARE, INC. OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include <iostream>
#include "analysis_tool.h"
#include "analyzer.h"
#include "reader/file_reader.h"
#ifdef HAS_ZLIB
#    include "reader/compressed_file_reader.h"
#endif
#include "common/utils.h"

analyzer_t::analyzer_t()
    : success(true)
    , trace_iter(NULL)
    , trace_end(NULL)
    , num_tools(0)
    , tools(NULL)
{
    /* Nothing else: child class needs to initialize. */
}

bool
analyzer_t::init_file_reader(const std::string &trace_file)
{
    if (trace_file.empty()) {
        ERRMSG("Trace file name is empty\n");
        return false;
    }
#ifdef HAS_ZLIB
    // Even if the file is uncompressed, zlib's gzip interface is faster than
    // file_reader_t's fstream in our measurements, so we always use it when
    // available.
    trace_iter = new compressed_file_reader_t(trace_file.c_str());
    trace_end = new compressed_file_reader_t();
#else
    trace_iter = new file_reader_t<std::ifstream *>(trace_file.c_str());
    trace_end = new file_reader_t<std::ifstream *>();
#endif
    return true;
}

analyzer_t::analyzer_t(const std::string &trace_file, analysis_tool_t **tools_in,
                       int num_tools_in)
    : success(true)
    , trace_iter(NULL)
    , trace_end(NULL)
    , num_tools(num_tools_in)
    , tools(tools_in)
{
    for (int i = 0; i < num_tools; ++i) {
        if (tools[i] == NULL || !*tools[i]) {
            success = false;
            error_string = "Tool is not successfully initialized";
            if (tools[i] != NULL)
                error_string += ": " + tools[i]->get_error_string();
            return;
        }
        std::string error = tools[i]->initialize();
        if (!error.empty()) {
            success = false;
            error_string = "Tool failed to initialize: " + error;
            return;
        }
    }
    if (!init_file_reader(trace_file))
        success = false;
}

analyzer_t::analyzer_t(const std::string &trace_file)
    : success(true)
    , trace_iter(NULL)
    , trace_end(NULL)
    , num_tools(0)
    , tools(NULL)
{
    if (!init_file_reader(trace_file))
        success = false;
}

analyzer_t::~analyzer_t()
{
    delete trace_iter;
    delete trace_end;
}

// Work around clang-format bug: no newline after return type for single-char operator.
// clang-format off
bool
analyzer_t::operator!()
// clang-format on
{
    return !success;
}

std::string
analyzer_t::get_error_string()
{
    return error_string;
}

bool
analyzer_t::start_reading()
{
    if (!trace_iter->init()) {
        ERRMSG("Failed to read from trace\n");
        return false;
    }
    return true;
}

bool
analyzer_t::run()
{
    if (!start_reading())
        return false;

    for (; *trace_iter != *trace_end; ++(*trace_iter)) {
        for (int i = 0; i < num_tools; ++i) {
            memref_t memref = **trace_iter;
            // We short-circuit and exit on an error to avoid confusion over
            // the results and avoid wasted continued work.
            if (!tools[i]->process_memref(memref)) {
                error_string = tools[i]->get_error_string();
                return false;
            }
        }
    }
    return true;
}

bool
analyzer_t::print_stats()
{
    for (int i = 0; i < num_tools; ++i) {
        if (!tools[i]->print_results()) {
            error_string = tools[i]->get_error_string();
            return false;
        }
        if (i + 1 < num_tools) {
            // Separate tool output.
            std::cerr << "\n=========================================================="
                         "=================\n";
        }
    }
    return true;
}

reader_t &
analyzer_t::begin()
{
    if (!start_reading())
        return *trace_end;
    return *trace_iter;
}

reader_t &
analyzer_t::end()
{
    return *trace_end;
}
