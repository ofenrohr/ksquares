// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: GameSequence.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "GameSequence.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace AlphaDots {
class GameSequenceDefaultTypeInternal {
public:
 ::google::protobuf::internal::ExplicitlyConstructed<GameSequence>
     _instance;
} _GameSequence_default_instance_;

namespace protobuf_GameSequence_2eproto {


namespace {

::google::protobuf::Metadata file_level_metadata[1];

}  // namespace

PROTOBUF_CONSTEXPR_VAR ::google::protobuf::internal::ParseTableField
    const TableStruct::entries[] GOOGLE_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  {0, 0, 0, ::google::protobuf::internal::kInvalidMask, 0, 0},
};

PROTOBUF_CONSTEXPR_VAR ::google::protobuf::internal::AuxillaryParseTableField
    const TableStruct::aux[] GOOGLE_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  ::google::protobuf::internal::AuxillaryParseTableField(),
};
PROTOBUF_CONSTEXPR_VAR ::google::protobuf::internal::ParseTable const
    TableStruct::schema[] GOOGLE_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  { NULL, NULL, 0, -1, -1, -1, -1, NULL, false },
};

const ::google::protobuf::uint32 TableStruct::offsets[] GOOGLE_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(GameSequence, _has_bits_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(GameSequence, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(GameSequence, width_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(GameSequence, height_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(GameSequence, game_),
  0,
  1,
  ~0u,
};
static const ::google::protobuf::internal::MigrationSchema schemas[] GOOGLE_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  { 0, 8, sizeof(GameSequence)},
};

static ::google::protobuf::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::google::protobuf::Message*>(&_GameSequence_default_instance_),
};

namespace {

void protobuf_AssignDescriptors() {
  AddDescriptors();
  ::google::protobuf::MessageFactory* factory = NULL;
  AssignDescriptors(
      "GameSequence.proto", schemas, file_default_instances, TableStruct::offsets, factory,
      file_level_metadata, NULL, NULL);
}

void protobuf_AssignDescriptorsOnce() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &protobuf_AssignDescriptors);
}

void protobuf_RegisterTypes(const ::std::string&) GOOGLE_ATTRIBUTE_COLD;
void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::internal::RegisterAllTypes(file_level_metadata, 1);
}

}  // namespace
void TableStruct::InitDefaultsImpl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::internal::InitProtobufDefaults();
  ::AlphaDots::protobuf_TrainingExample_2eproto::InitDefaults();
  _GameSequence_default_instance_._instance.DefaultConstruct();
  ::google::protobuf::internal::OnShutdownDestroyMessage(
      &_GameSequence_default_instance_);}

void InitDefaults() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &TableStruct::InitDefaultsImpl);
}
namespace {
void AddDescriptorsImpl() {
  InitDefaults();
  static const char descriptor[] GOOGLE_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
      "\n\022GameSequence.proto\022\tAlphaDots\032\025Trainin"
      "gExample.proto\"W\n\014GameSequence\022\r\n\005width\030"
      "\001 \002(\005\022\016\n\006height\030\002 \002(\005\022(\n\004game\030\004 \003(\0132\032.Al"
      "phaDots.TrainingExample"
  };
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
      descriptor, 143);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "GameSequence.proto", &protobuf_RegisterTypes);
  ::AlphaDots::protobuf_TrainingExample_2eproto::AddDescriptors();
}
} // anonymous namespace

void AddDescriptors() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &AddDescriptorsImpl);
}
// Force AddDescriptors() to be called at dynamic initialization time.
struct StaticDescriptorInitializer {
  StaticDescriptorInitializer() {
    AddDescriptors();
  }
} static_descriptor_initializer;

}  // namespace protobuf_GameSequence_2eproto


// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int GameSequence::kWidthFieldNumber;
const int GameSequence::kHeightFieldNumber;
const int GameSequence::kGameFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

GameSequence::GameSequence()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  if (GOOGLE_PREDICT_TRUE(this != internal_default_instance())) {
    protobuf_GameSequence_2eproto::InitDefaults();
  }
  SharedCtor();
  // @@protoc_insertion_point(constructor:AlphaDots.GameSequence)
}
GameSequence::GameSequence(const GameSequence& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL),
      _has_bits_(from._has_bits_),
      _cached_size_(0),
      game_(from.game_) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::memcpy(&width_, &from.width_,
    static_cast<size_t>(reinterpret_cast<char*>(&height_) -
    reinterpret_cast<char*>(&width_)) + sizeof(height_));
  // @@protoc_insertion_point(copy_constructor:AlphaDots.GameSequence)
}

void GameSequence::SharedCtor() {
  _cached_size_ = 0;
  ::memset(&width_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&height_) -
      reinterpret_cast<char*>(&width_)) + sizeof(height_));
}

GameSequence::~GameSequence() {
  // @@protoc_insertion_point(destructor:AlphaDots.GameSequence)
  SharedDtor();
}

void GameSequence::SharedDtor() {
}

void GameSequence::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* GameSequence::descriptor() {
  protobuf_GameSequence_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_GameSequence_2eproto::file_level_metadata[kIndexInFileMessages].descriptor;
}

const GameSequence& GameSequence::default_instance() {
  protobuf_GameSequence_2eproto::InitDefaults();
  return *internal_default_instance();
}

GameSequence* GameSequence::New(::google::protobuf::Arena* arena) const {
  GameSequence* n = new GameSequence;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void GameSequence::Clear() {
// @@protoc_insertion_point(message_clear_start:AlphaDots.GameSequence)
  ::google::protobuf::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  game_.Clear();
  cached_has_bits = _has_bits_[0];
  if (cached_has_bits & 3u) {
    ::memset(&width_, 0, static_cast<size_t>(
        reinterpret_cast<char*>(&height_) -
        reinterpret_cast<char*>(&width_)) + sizeof(height_));
  }
  _has_bits_.Clear();
  _internal_metadata_.Clear();
}

bool GameSequence::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:AlphaDots.GameSequence)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required int32 width = 1;
      case 1: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(8u /* 8 & 0xFF */)) {
          set_has_width();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &width_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // required int32 height = 2;
      case 2: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(16u /* 16 & 0xFF */)) {
          set_has_height();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &height_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // repeated .AlphaDots.TrainingExample game = 4;
      case 4: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(34u /* 34 & 0xFF */)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessageNoVirtual(
                input, add_game()));
        } else {
          goto handle_unusual;
        }
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, _internal_metadata_.mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:AlphaDots.GameSequence)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:AlphaDots.GameSequence)
  return false;
#undef DO_
}

void GameSequence::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:AlphaDots.GameSequence)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // required int32 width = 1;
  if (cached_has_bits & 0x00000001u) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(1, this->width(), output);
  }

  // required int32 height = 2;
  if (cached_has_bits & 0x00000002u) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(2, this->height(), output);
  }

  // repeated .AlphaDots.TrainingExample game = 4;
  for (unsigned int i = 0,
      n = static_cast<unsigned int>(this->game_size()); i < n; i++) {
    ::google::protobuf::internal::WireFormatLite::WriteMessageMaybeToArray(
      4, this->game(static_cast<int>(i)), output);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        _internal_metadata_.unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:AlphaDots.GameSequence)
}

::google::protobuf::uint8* GameSequence::InternalSerializeWithCachedSizesToArray(
    bool deterministic, ::google::protobuf::uint8* target) const {
  (void)deterministic; // Unused
  // @@protoc_insertion_point(serialize_to_array_start:AlphaDots.GameSequence)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // required int32 width = 1;
  if (cached_has_bits & 0x00000001u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(1, this->width(), target);
  }

  // required int32 height = 2;
  if (cached_has_bits & 0x00000002u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(2, this->height(), target);
  }

  // repeated .AlphaDots.TrainingExample game = 4;
  for (unsigned int i = 0,
      n = static_cast<unsigned int>(this->game_size()); i < n; i++) {
    target = ::google::protobuf::internal::WireFormatLite::
      InternalWriteMessageNoVirtualToArray(
        4, this->game(static_cast<int>(i)), deterministic, target);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:AlphaDots.GameSequence)
  return target;
}

size_t GameSequence::RequiredFieldsByteSizeFallback() const {
// @@protoc_insertion_point(required_fields_byte_size_fallback_start:AlphaDots.GameSequence)
  size_t total_size = 0;

  if (has_width()) {
    // required int32 width = 1;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::Int32Size(
        this->width());
  }

  if (has_height()) {
    // required int32 height = 2;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::Int32Size(
        this->height());
  }

  return total_size;
}
size_t GameSequence::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:AlphaDots.GameSequence)
  size_t total_size = 0;

  if (_internal_metadata_.have_unknown_fields()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        _internal_metadata_.unknown_fields());
  }
  if (((_has_bits_[0] & 0x00000003) ^ 0x00000003) == 0) {  // All required fields are present.
    // required int32 width = 1;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::Int32Size(
        this->width());

    // required int32 height = 2;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::Int32Size(
        this->height());

  } else {
    total_size += RequiredFieldsByteSizeFallback();
  }
  // repeated .AlphaDots.TrainingExample game = 4;
  {
    unsigned int count = static_cast<unsigned int>(this->game_size());
    total_size += 1UL * count;
    for (unsigned int i = 0; i < count; i++) {
      total_size +=
        ::google::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
          this->game(static_cast<int>(i)));
    }
  }

  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = cached_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void GameSequence::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:AlphaDots.GameSequence)
  GOOGLE_DCHECK_NE(&from, this);
  const GameSequence* source =
      ::google::protobuf::internal::DynamicCastToGenerated<const GameSequence>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:AlphaDots.GameSequence)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:AlphaDots.GameSequence)
    MergeFrom(*source);
  }
}

void GameSequence::MergeFrom(const GameSequence& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:AlphaDots.GameSequence)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  game_.MergeFrom(from.game_);
  cached_has_bits = from._has_bits_[0];
  if (cached_has_bits & 3u) {
    if (cached_has_bits & 0x00000001u) {
      width_ = from.width_;
    }
    if (cached_has_bits & 0x00000002u) {
      height_ = from.height_;
    }
    _has_bits_[0] |= cached_has_bits;
  }
}

void GameSequence::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:AlphaDots.GameSequence)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void GameSequence::CopyFrom(const GameSequence& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:AlphaDots.GameSequence)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool GameSequence::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000003) != 0x00000003) return false;
  if (!::google::protobuf::internal::AllAreInitialized(this->game())) return false;
  return true;
}

void GameSequence::Swap(GameSequence* other) {
  if (other == this) return;
  InternalSwap(other);
}
void GameSequence::InternalSwap(GameSequence* other) {
  using std::swap;
  game_.InternalSwap(&other->game_);
  swap(width_, other->width_);
  swap(height_, other->height_);
  swap(_has_bits_[0], other->_has_bits_[0]);
  _internal_metadata_.Swap(&other->_internal_metadata_);
  swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata GameSequence::GetMetadata() const {
  protobuf_GameSequence_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_GameSequence_2eproto::file_level_metadata[kIndexInFileMessages];
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// GameSequence

// required int32 width = 1;
bool GameSequence::has_width() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
void GameSequence::set_has_width() {
  _has_bits_[0] |= 0x00000001u;
}
void GameSequence::clear_has_width() {
  _has_bits_[0] &= ~0x00000001u;
}
void GameSequence::clear_width() {
  width_ = 0;
  clear_has_width();
}
::google::protobuf::int32 GameSequence::width() const {
  // @@protoc_insertion_point(field_get:AlphaDots.GameSequence.width)
  return width_;
}
void GameSequence::set_width(::google::protobuf::int32 value) {
  set_has_width();
  width_ = value;
  // @@protoc_insertion_point(field_set:AlphaDots.GameSequence.width)
}

// required int32 height = 2;
bool GameSequence::has_height() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
void GameSequence::set_has_height() {
  _has_bits_[0] |= 0x00000002u;
}
void GameSequence::clear_has_height() {
  _has_bits_[0] &= ~0x00000002u;
}
void GameSequence::clear_height() {
  height_ = 0;
  clear_has_height();
}
::google::protobuf::int32 GameSequence::height() const {
  // @@protoc_insertion_point(field_get:AlphaDots.GameSequence.height)
  return height_;
}
void GameSequence::set_height(::google::protobuf::int32 value) {
  set_has_height();
  height_ = value;
  // @@protoc_insertion_point(field_set:AlphaDots.GameSequence.height)
}

// repeated .AlphaDots.TrainingExample game = 4;
int GameSequence::game_size() const {
  return game_.size();
}
void GameSequence::clear_game() {
  game_.Clear();
}
const ::AlphaDots::TrainingExample& GameSequence::game(int index) const {
  // @@protoc_insertion_point(field_get:AlphaDots.GameSequence.game)
  return game_.Get(index);
}
::AlphaDots::TrainingExample* GameSequence::mutable_game(int index) {
  // @@protoc_insertion_point(field_mutable:AlphaDots.GameSequence.game)
  return game_.Mutable(index);
}
::AlphaDots::TrainingExample* GameSequence::add_game() {
  // @@protoc_insertion_point(field_add:AlphaDots.GameSequence.game)
  return game_.Add();
}
::google::protobuf::RepeatedPtrField< ::AlphaDots::TrainingExample >*
GameSequence::mutable_game() {
  // @@protoc_insertion_point(field_mutable_list:AlphaDots.GameSequence.game)
  return &game_;
}
const ::google::protobuf::RepeatedPtrField< ::AlphaDots::TrainingExample >&
GameSequence::game() const {
  // @@protoc_insertion_point(field_list:AlphaDots.GameSequence.game)
  return game_;
}

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)

}  // namespace AlphaDots

// @@protoc_insertion_point(global_scope)