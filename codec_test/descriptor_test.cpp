//
// Created by skyitachi on 2018/9/24.
//

#include "query.pb.h"
#include "protobuf_codec.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <string>

using std::cout;
using std::endl;

using google::protobuf::Descriptor;
using google::protobuf::DescriptorPool;

template <typename T>
void testDescriptor() {
  std::string type_name = T::descriptor()->full_name();
  cout << type_name << endl;

  const Descriptor *descriptor = DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);

  assert(descriptor == T::descriptor());

  cout << "FindMessageByTypeName() = " << descriptor << endl;

  const google::protobuf::Message *prototype =
      google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
  assert(prototype == &T::default_instance());

  T *new_obj = dynamic_cast<T*>(prototype->New());
  assert(new_obj != NULL);
  assert(new_obj != prototype);
  assert(typeid(*new_obj) == typeid(T::default_instance()));
  delete new_obj;
}

int main() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  testDescriptor<learnuv::Query>();
  testDescriptor<learnuv::Answer>();

  google::protobuf::Message* newQuery = createMessage("learnuv.Query");
  assert(newQuery != NULL);
  assert(typeid(*newQuery) == typeid(learnuv::Query::default_instance()));

  cout << "createMessage(\"learnuv::Query\") = " << newQuery << endl;
  delete newQuery;
}
