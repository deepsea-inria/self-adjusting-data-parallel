
#include <iostream>

#include "trace.hpp"

std::ostream& operator<<(std::ostream& out, const modseq_type& modseq) {
  return pasl::data::chunkedseq::extras::generic_print_container(out, modseq.seq);
}

std::ostream& operator<<(std::ostream& out, const sequence_type& seq) {
  return pasl::data::chunkedseq::extras::generic_print_container(out, seq);
}

std::ostream& operator<<(std::ostream& out, const concat_tree_node_type& n) {
  if (! n.modseq) {
    return out;
  }
  const modseq_type& modseq = *n.modseq.get();
  return pasl::data::chunkedseq::extras::generic_print_container(out, modseq.seq);
}

class foo {
public:
  virtual void f() = 0;
};

class bar : public foo {
public:
  
  int x;
  
  void foo() {
    
  }
  
};

int main(int argc, const char * argv[]) {
  sequence_type newseq = {32,33};
  sequence_changes_type changes = {
                mk_sequence_change_insert(newseq),
    mk_sequence_change_keep(2),
    mk_sequence_change_remove(2),
    mk_sequence_change_keep(2),
    mk_sequence_change_remove(1)
  };
  sequence_type source = { 1, 2, 3, 4, 5, 6, 7 };
  sequence_type destination;
  apply_sequence_changes(changes, source, destination);
  std::cout << "source = " << source << std::endl;
  std::cout << "destination = " << destination << std::endl;
  
  std::vector<std::unique_ptr<foo>> vec;
  vec.resize(2);
  
  return 0;
}
