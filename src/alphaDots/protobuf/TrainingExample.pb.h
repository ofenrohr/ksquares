// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: TrainingExample.proto

#ifndef PROTOBUF_TrainingExample_2eproto__INCLUDED
#define PROTOBUF_TrainingExample_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3004000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3004000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
namespace AlphaDots {
class TrainingExample;
class TrainingExampleDefaultTypeInternal;
extern TrainingExampleDefaultTypeInternal _TrainingExample_default_instance_;
}  // namespace AlphaDots

namespace AlphaDots {

namespace protobuf_TrainingExample_2eproto {
// Internal implementation detail -- do not call these.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[];
  static const ::google::protobuf::uint32 offsets[];
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static void InitDefaultsImpl();
};
void AddDescriptors();
void InitDefaults();
}  // namespace protobuf_TrainingExample_2eproto

// ===================================================================

class TrainingExample : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:AlphaDots.TrainingExample) */ {
 public:
  TrainingExample();
  virtual ~TrainingExample();

  TrainingExample(const TrainingExample& from);

  inline TrainingExample& operator=(const TrainingExample& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  TrainingExample(TrainingExample&& from) noexcept
    : TrainingExample() {
    *this = ::std::move(from);
  }

  inline TrainingExample& operator=(TrainingExample&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const TrainingExample& default_instance();

  static inline const TrainingExample* internal_default_instance() {
    return reinterpret_cast<const TrainingExample*>(
               &_TrainingExample_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(TrainingExample* other);
  friend void swap(TrainingExample& a, TrainingExample& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline TrainingExample* New() const PROTOBUF_FINAL { return New(NULL); }

  TrainingExample* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const TrainingExample& from);
  void MergeFrom(const TrainingExample& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(TrainingExample* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // repeated int32 input = 3;
  int input_size() const;
  void clear_input();
  static const int kInputFieldNumber = 3;
  ::google::protobuf::int32 input(int index) const;
  void set_input(int index, ::google::protobuf::int32 value);
  void add_input(::google::protobuf::int32 value);
  const ::google::protobuf::RepeatedField< ::google::protobuf::int32 >&
      input() const;
  ::google::protobuf::RepeatedField< ::google::protobuf::int32 >*
      mutable_input();

  // repeated int32 output = 4;
  int output_size() const;
  void clear_output();
  static const int kOutputFieldNumber = 4;
  ::google::protobuf::int32 output(int index) const;
  void set_output(int index, ::google::protobuf::int32 value);
  void add_output(::google::protobuf::int32 value);
  const ::google::protobuf::RepeatedField< ::google::protobuf::int32 >&
      output() const;
  ::google::protobuf::RepeatedField< ::google::protobuf::int32 >*
      mutable_output();

  // required int32 width = 1;
  bool has_width() const;
  void clear_width();
  static const int kWidthFieldNumber = 1;
  ::google::protobuf::int32 width() const;
  void set_width(::google::protobuf::int32 value);

  // required int32 height = 2;
  bool has_height() const;
  void clear_height();
  static const int kHeightFieldNumber = 2;
  ::google::protobuf::int32 height() const;
  void set_height(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:AlphaDots.TrainingExample)
 private:
  void set_has_width();
  void clear_has_width();
  void set_has_height();
  void clear_has_height();

  // helper for ByteSizeLong()
  size_t RequiredFieldsByteSizeFallback() const;

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::HasBits<1> _has_bits_;
  mutable int _cached_size_;
  ::google::protobuf::RepeatedField< ::google::protobuf::int32 > input_;
  ::google::protobuf::RepeatedField< ::google::protobuf::int32 > output_;
  ::google::protobuf::int32 width_;
  ::google::protobuf::int32 height_;
  friend struct protobuf_TrainingExample_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// TrainingExample

// required int32 width = 1;
inline bool TrainingExample::has_width() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void TrainingExample::set_has_width() {
  _has_bits_[0] |= 0x00000001u;
}
inline void TrainingExample::clear_has_width() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void TrainingExample::clear_width() {
  width_ = 0;
  clear_has_width();
}
inline ::google::protobuf::int32 TrainingExample::width() const {
  // @@protoc_insertion_point(field_get:AlphaDots.TrainingExample.width)
  return width_;
}
inline void TrainingExample::set_width(::google::protobuf::int32 value) {
  set_has_width();
  width_ = value;
  // @@protoc_insertion_point(field_set:AlphaDots.TrainingExample.width)
}

// required int32 height = 2;
inline bool TrainingExample::has_height() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void TrainingExample::set_has_height() {
  _has_bits_[0] |= 0x00000002u;
}
inline void TrainingExample::clear_has_height() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void TrainingExample::clear_height() {
  height_ = 0;
  clear_has_height();
}
inline ::google::protobuf::int32 TrainingExample::height() const {
  // @@protoc_insertion_point(field_get:AlphaDots.TrainingExample.height)
  return height_;
}
inline void TrainingExample::set_height(::google::protobuf::int32 value) {
  set_has_height();
  height_ = value;
  // @@protoc_insertion_point(field_set:AlphaDots.TrainingExample.height)
}

// repeated int32 input = 3;
inline int TrainingExample::input_size() const {
  return input_.size();
}
inline void TrainingExample::clear_input() {
  input_.Clear();
}
inline ::google::protobuf::int32 TrainingExample::input(int index) const {
  // @@protoc_insertion_point(field_get:AlphaDots.TrainingExample.input)
  return input_.Get(index);
}
inline void TrainingExample::set_input(int index, ::google::protobuf::int32 value) {
  input_.Set(index, value);
  // @@protoc_insertion_point(field_set:AlphaDots.TrainingExample.input)
}
inline void TrainingExample::add_input(::google::protobuf::int32 value) {
  input_.Add(value);
  // @@protoc_insertion_point(field_add:AlphaDots.TrainingExample.input)
}
inline const ::google::protobuf::RepeatedField< ::google::protobuf::int32 >&
TrainingExample::input() const {
  // @@protoc_insertion_point(field_list:AlphaDots.TrainingExample.input)
  return input_;
}
inline ::google::protobuf::RepeatedField< ::google::protobuf::int32 >*
TrainingExample::mutable_input() {
  // @@protoc_insertion_point(field_mutable_list:AlphaDots.TrainingExample.input)
  return &input_;
}

// repeated int32 output = 4;
inline int TrainingExample::output_size() const {
  return output_.size();
}
inline void TrainingExample::clear_output() {
  output_.Clear();
}
inline ::google::protobuf::int32 TrainingExample::output(int index) const {
  // @@protoc_insertion_point(field_get:AlphaDots.TrainingExample.output)
  return output_.Get(index);
}
inline void TrainingExample::set_output(int index, ::google::protobuf::int32 value) {
  output_.Set(index, value);
  // @@protoc_insertion_point(field_set:AlphaDots.TrainingExample.output)
}
inline void TrainingExample::add_output(::google::protobuf::int32 value) {
  output_.Add(value);
  // @@protoc_insertion_point(field_add:AlphaDots.TrainingExample.output)
}
inline const ::google::protobuf::RepeatedField< ::google::protobuf::int32 >&
TrainingExample::output() const {
  // @@protoc_insertion_point(field_list:AlphaDots.TrainingExample.output)
  return output_;
}
inline ::google::protobuf::RepeatedField< ::google::protobuf::int32 >*
TrainingExample::mutable_output() {
  // @@protoc_insertion_point(field_mutable_list:AlphaDots.TrainingExample.output)
  return &output_;
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)


}  // namespace AlphaDots

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_TrainingExample_2eproto__INCLUDED
