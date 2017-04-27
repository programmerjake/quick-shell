#!/bin/qsh
#
# Copyright 2017 Jacob Lifshay
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

function simple_js_get_token()
{
    simple_js_token="${simple_js_input:simple_js_input_location++:1}"
    if [[ "$simple_js_token" =~ [^]()[!+] ]]; then
        echo "simple js parse error: invalid token: '$simple_js_token'" >&2
        exit 1
    fi
    #echo "get_token: '$simple_js_token'" >&2
}

function simple_js_to_string()
{
    simple_js_to_primitive
    if [[ "$simple_js_result_type" == 'B' ]]; then
        if (( "$simple_js_result" )); then
            simple_js_result=true
        else
            simple_js_result=false
        fi
    fi
    simple_js_result_type=S
    #echo "to_string: $simple_js_result_type '$simple_js_result'" >&2
}

function simple_js_to_primitive()
{
    if [[ "$simple_js_result_type" == 'O' ]]; then
        simple_js_result=
        simple_js_result_type=S
    fi
    #echo "to_primitive: $simple_js_result_type '$simple_js_result'" >&2
}

function simple_js_to_number()
{
    simple_js_to_primitive
    if [[ "$simple_js_result_type" == 'S' ]]; then
        if [[ -z "$simple_js_result" ]]; then
            simple_js_result=0
        elif [[ "$simple_js_result" =~ ^[-+]?[0-9][0-9]*$ ]]; then
            simple_js_result=$((simple_js_result))
        else
            simple_js_result=0
        fi
    fi
    simple_js_result_type=N
    #echo "to_number: $simple_js_result_type '$simple_js_result'" >&2
}

function simple_js_to_boolean()
{
    case "$simple_js_result_type" in
    O)
        simple_js_result=1
    ;;
    N)
        if (( simple_js_result )); then
            simple_js_result=1
        else
            simple_js_result=0
        fi
    ;;
    S)
        if [[ -z "$simple_js_result" ]]; then
            simple_js_result=0
        else
            simple_js_result=1
        fi
    ;;
    esac
    simple_js_result_type=B
    #echo "to_boolean: $simple_js_result_type '$simple_js_result'" >&2
}

function simple_js_top_level_expression()
{
    #echo "enter top_level_expression" >&2
    case "$simple_js_token" in
    \[)
        simple_js_get_token
        if [[ "$simple_js_token" != ']' ]]; then
            echo "simple js parse error: missing ]" >&2
            exit 1
        fi
        simple_js_get_token
        simple_js_result=
        simple_js_result_type=O
    ;;
    \()
        simple_js_get_token
        simple_js_expression
        if [[ "$simple_js_token" != ')' ]]; then
            echo "simple js parse error: missing )" >&2
            exit 1
        fi
        simple_js_get_token
    ;;
    *)
        echo "simple js parse error: unexpected token: '$simple_js_token'" >&2
        exit 1
    ;;
    esac
    #echo "exit top_level_expression: $simple_js_result_type '$simple_js_result'" >&2
}

function simple_js_unary_expression()
{
    #echo "enter unary_expression" >&2
    case "$simple_js_token" in
    \+)
        simple_js_get_token
        simple_js_unary_expression
        simple_js_to_number
    ;;
    \!)
        simple_js_get_token
        simple_js_unary_expression
        simple_js_to_boolean
        (( simple_js_result = !simple_js_result ))
    ;;
    *)
        simple_js_top_level_expression
    ;;
    esac
    #echo "exit unary_expression: $simple_js_result_type '$simple_js_result'" >&2
}

function simple_js_add_expression()
{
    #echo "enter add_expression" >&2
    local v v_type sv nv
    simple_js_unary_expression
    while [[ "$simple_js_token" == '+' ]]; do
        simple_js_to_primitive
        v="$simple_js_result"
        v_type="$simple_js_result_type"
        simple_js_to_string
        sv="$simple_js_result"
        simple_js_result="$v"
        simple_js_result_type="$v_type"
        simple_js_to_number
        nv="$simple_js_result"
        simple_js_get_token
        simple_js_unary_expression
        simple_js_to_primitive
        if [[ "$simple_js_result_type" == 'S' || "$v_type" == 'S' ]]; then
            simple_js_to_string
            simple_js_result="$sv$simple_js_result"
            simple_js_result_type='S'
        else
            simple_js_to_number
            (( simple_js_result += nv ))
            simple_js_result_type='N'
        fi
        #echo "in add_expression: $simple_js_result_type '$simple_js_result'" >&2
    done
    #echo "exit add_expression: $simple_js_result_type '$simple_js_result'" >&2
}

function simple_js_expression()
{
    simple_js_add_expression
}

function simple_js_eval()
{
    (simple_js_input="$1"; simple_js_input_location=0; simple_js_get_token; simple_js_expression; echo "$simple_js_result")
}

