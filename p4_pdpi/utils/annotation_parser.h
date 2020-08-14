/*
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _COMMON_ANNOTATION_PARSER_H_
#define _COMMON_ANNOTATION_PARSER_H_

#include <functional>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "gutil/status.h"
#include "re2/re2.h"

// This header file defines libraries for parsing P4 annotations.
//
// Valid P4 annotations have the following format:
//   @<label>(<body>)
// Examples:
//   @sai_acl(INGRESS)
//   $sai_action(SAI_PACKET_ACTION_DROP, RED)
//
// P4 annotations ignore spaces within the parentheses and between the label and
// first parenthesis. For example,
//   @sai_acl(INGRESS) is treated the same as @sai_acl (  INGRESS )

namespace pdpi {

namespace annotation {
// Template type definition of a parser function to be used in GetAnnotation and
// GetAllAnnotations. The parser function will be called with the body of a
// matching annotation.
template <typename T>
using BodyParser = std::function<gutil::StatusOr<T>(std::string)>;

// Parses a string of the format: "arg [, arg2] [, arg3] [, ...]" into separate,
// ordered arguments. Returned arguments are stripped of whitespace.
// Returns an error if the string contains any character that is neither
// alphanumeric, comma, space, tab, nor underscore.
gutil::StatusOr<std::vector<std::string>> ParseAsArgList(std::string body);

// Returns the raw input string.
inline gutil::StatusOr<std::string> Raw(std::string body) { return body; }

namespace internal {
// Structure to hold the useful components of an annotation.
struct AnnotationComponents {
  std::string label;
  std::string body;
};

// Parses an annotation into the AnnotationComponents.
// Returns an InvalidArgument error if parsing failed.
gutil::StatusOr<AnnotationComponents> ParseAnnotation(
    const std::string& annotation);

}  // namespace internal
}  // namespace annotation

// Returns a list of the parsed body of all annotations with the given label.
// Returns a Status with code kNotFound if there is no matching annotation.
//
// Note: Currently, template type deduction does not work for this function. To
// use GetAllParsedAnnotations, declare it with the parser's StatusOr<T> T type.
// Example:
//   gutil::StatusOr<int>(std::string> ps) parser;
//   ASSIGN_OR_RETURN(auto result,
//                    GetAllParsedAnnotations<int>(label, annotations, parser);
template <typename T, typename Container>
gutil::StatusOr<std::vector<T>> GetAllParsedAnnotations(
    absl::string_view label, const Container& annotations,
    annotation::BodyParser<T> parser) {
  std::vector<T> values;
  for (const auto& annotation : annotations) {
    auto parser_result = annotation::internal::ParseAnnotation(annotation);
    if (!parser_result.ok()) continue;  // Skip unknown labels.

    const annotation::internal::AnnotationComponents& parsed_annotation =
        parser_result.value();
    if (parsed_annotation.label == label) {
      ASSIGN_OR_RETURN(
          T value, parser(std::move(parsed_annotation.body)),
          _ << "Failed to parse annotation \"" << annotation << "\".");
      values.push_back(value);
    }
  }
  if (values.empty()) {
    return gutil::NotFoundErrorBuilder()
           << "No annotation contained label \"" << label << "\".";
  }
  return values;
}

// Returns the parsed body of the unique annotation with the given label.
// Returns a Status with code kNotFound if there is no matching annotation.
// Returns a Status with code kInvalidArgument if there are multiple matching
// annotations.
//
// Note: Currently, template type deduction does not work for this function. To
// use GetAnnotation, declare it with the parser's StatusOr<T> T type.
// Example:
//   gutil::StatusOr<int>(std::string> s) parser;
//   ASSIGN_OR_RETURN(int result,
//                    GetParsedAnnotation<int>(label, annotations, parser);
template <typename T, typename Container>
gutil::StatusOr<T> GetParsedAnnotation(absl::string_view label,
                                       const Container& annotations,
                                       annotation::BodyParser<T> parser) {
  // Add extra parentheses around GetAllParsedAnnotations to prevent the
  // precompiler from splitting GetAllParsedAnnotations<T into its own argument.
  ASSIGN_OR_RETURN(auto values, (GetAllParsedAnnotations<T, Container>(
                                    label, annotations, parser)));
  if (values.size() > 1) {
    return gutil::InvalidArgumentErrorBuilder()
           << "Multiple annotations contained label \"" << label << "\".";
  }
  return values[0];
}

// Returns the body of an annotation with the unique label as a list of strings.
// Returns an empty list if the matching annotation has no arguments.
// Returns a Status with code kNotFound if there is no matching annotation.
template <typename Container>
gutil::StatusOr<std::vector<std::string>> GetAnnotationAsArgList(
    absl::string_view label, const Container& annotations) {
  return GetParsedAnnotation<std::vector<std::string>>(
      label, annotations, annotation::ParseAsArgList);
}

// Returns the string list form of the body of all annotations with the given
// label.
// Returns a Status with code kNotFound if there is no matching annotation.
template <typename Container>
gutil::StatusOr<std::vector<std::vector<std::string>>>
GetAllAnnotationsAsArgList(absl::string_view label,
                           const Container& annotations) {
  return GetAllParsedAnnotations<std::vector<std::string>>(
      label, annotations, annotation::ParseAsArgList);
}

// Returns the body of an annotation with the given label.
// Returns a Status with code kNotFound if there is no matching annotation.
template <typename Container>
gutil::StatusOr<std::string> GetAnnotationBody(absl::string_view label,
                                               const Container& annotations) {
  return GetParsedAnnotation<std::string>(label, annotations, annotation::Raw);
}

// Returns all annotation bodies from all annotations with the given label.
// Returns a Status with code kNotFound if there is no matching annotation.
template <typename Container>
gutil::StatusOr<std::vector<std::string>> GetAllAnnotationBodies(
    absl::string_view label, const Container& annotations) {
  return GetAllParsedAnnotations<std::string>(label, annotations,
                                              annotation::Raw);
}

}  // namespace pdpi

#endif  // _COMMON_ANNOTATION_PARSER_H_