
#include "chunkedseq.hpp"

#ifndef _SACDP_TRACE_H_
#define _SACDP_TRACE_H_

using version_number_type = size_t;

static constexpr
int chunk_capacity = 512;

template <class Item>
using deque_type = pasl::data::chunkedseq::bootstrapped::deque<Item, chunk_capacity>;

using sequence_type = deque_type<int>;

class modseq_type {
public:
  
  sequence_type seq;
  
};

version_number_type current_version = 1;

class concat_tree_node_type {
public:
  
  std::unique_ptr<modseq_type> modseq;
  size_t size = 0;
  concat_tree_node_type* parent = nullptr;
  concat_tree_node_type* branches[2] = { nullptr, nullptr };
  concat_tree_node_type* old_branches[2] = { nullptr, nullptr };
  concat_tree_node_type* new_branches[2] = { nullptr, nullptr };
  version_number_type version = current_version - 1;

  void update_size() {
    assert(modseq);
    size = modseq->seq.size();
  }
  
  using tag_type = enum { leaf_tag, interior_tag, freeze_tag };
  
  tag_type get_tag() {
    if (parent == nullptr) {
      return freeze_tag;
    } else if (branches[1] == nullptr) {
      return leaf_tag;
    } else {
      return interior_tag;
    }
  }
  
  void check() {
    tag_type tag = get_tag();
    assert(branches[0] != nullptr);
    if (tag == leaf_tag) {
      assert(branches[1] == nullptr);
      assert(parent != nullptr);
    } else if (tag == interior_tag) {
      assert(branches[1] != nullptr);
      assert(parent != nullptr);
    } else if (tag == freeze_tag) {
      assert(parent == nullptr);
      assert(size == modseq->seq.size());
    } else {
      assert(false);
    }
    assert(old_branches[0] == nullptr && old_branches[1] == nullptr);
    assert(new_branches[0] == nullptr && new_branches[1] == nullptr);
  }
  
};

concat_tree_node_type* mk_concat_tree_leaf(int val) {
  concat_tree_node_type* n = new concat_tree_node_type;
  n->modseq.reset(new modseq_type);
  n->modseq->seq.push_back(val);
  n->update_size();
  return n;
}

concat_tree_node_type* mk_concat_tree_interior(concat_tree_node_type* branch1,
                                               concat_tree_node_type* branch2) {
  concat_tree_node_type* n = new concat_tree_node_type;
  n->modseq.reset(new modseq_type);
  assert(branch1->modseq && branch2->modseq);
  n->modseq->seq.swap(branch1->modseq->seq);
  n->modseq->seq.concat(branch2->modseq->seq);
  n->branches[0] = branch1;
  n->branches[1] = branch2;
  branch1->parent = n;
  branch2->parent = n;
  n->update_size();
  return n;
}

concat_tree_node_type* mk_concat_tree_freeze(concat_tree_node_type* root) {
  concat_tree_node_type* n = new concat_tree_node_type;
  n->modseq.reset(new modseq_type);
  n->modseq->seq.swap(root->modseq->seq);
  n->branches[0] = root;
  root->parent = n;
  n->update_size();
  return n;
}

void concat_tree_update_version_number(concat_tree_node_type* n) {
  while (n != nullptr) {
    auto& v = n->version;
    if (v == current_version) {
      break;
    }
    v = current_version;
    n = n->parent;
  }
}

int concat_tree_branch_of(concat_tree_node_type* parent, concat_tree_node_type* branch) {
  if (parent->branches[0] == branch) {
    return 0;
  } else if (parent->branches[1] == branch) {
    return 1;
  } else {
    assert(false);
    return -1;
  }
}

void concat_tree_remove_node(concat_tree_node_type* n) {
  concat_tree_node_type* parent = n->parent;
  if (parent == nullptr) {
    return;
  }
  concat_tree_update_version_number(n);
  int i = concat_tree_branch_of(parent, n);
  assert(parent->branches[i] == n);
  assert(parent->old_branches[i] == nullptr);
  assert(parent->new_branches[i] == nullptr);
  parent->old_branches[i] = parent->branches[i];
}

void concat_tree_insert_node(concat_tree_node_type* n, concat_tree_node_type* b, int i) {
  assert(n->new_branches[i] == nullptr);
  concat_tree_update_version_number(n);
  b->parent = n;
  n->new_branches[i] = b;
}

class sequence_change_type {
public:
  
  enum { keep_tag, remove_tag, insert_tag, bogus_tag } tag = bogus_tag;
  
  size_t nb_to_keep = 0;
    
  size_t nb_to_remove = 0;
    
  sequence_type* items_to_insert = nullptr;

};

sequence_change_type mk_sequence_change_keep(size_t n) {
  sequence_change_type c;
  c.tag = sequence_change_type::keep_tag;
  c.nb_to_keep = n;
  return c;
}

sequence_change_type mk_sequence_change_remove(size_t n) {
  sequence_change_type c;
  c.tag = sequence_change_type::remove_tag;
  c.nb_to_remove = n;
  return c;
}

sequence_change_type mk_sequence_change_insert(sequence_type& s) {
  sequence_change_type c;
  c.tag = sequence_change_type::insert_tag;
  c.items_to_insert = new sequence_type;
  c.items_to_insert->swap(s);
  return c;
}

using sequence_changes_type = deque_type<sequence_change_type>;

void sequence_change_combine(sequence_change_type& c1,
                             sequence_change_type& c2,
                             sequence_changes_type& destination) {
  auto t1 = c1.tag;
  auto t2 = c2.tag;
  if (t1 != t2) {
    destination = {c1, c2};
    return;
  }
  if (t1 == sequence_change_type::keep_tag) {
    assert(false); // todo
  } else if (t1 == sequence_change_type::remove_tag) {
    assert(false); // todo
  } else if (t1 == sequence_change_type::insert_tag) {
    assert(false); // todo
  } else {
    assert(false);
  }
}

size_t sequence_changes_nb_to_keep(const sequence_changes_type& changes) {
  size_t r = 0;
  for (auto& c : changes) {
    if (c.tag == sequence_change_type::keep_tag) {
      r += c.nb_to_keep;
    }
  }
  return r;
}

size_t sequence_changes_nb_to_remove(const sequence_changes_type& changes) {
  size_t r = 0;
  for (auto& c : changes) {
    if (c.tag == sequence_change_type::remove_tag) {
      r += c.nb_to_remove;
    }
  }
  return r;
}

size_t sequence_changes_nb_to_insert(const sequence_changes_type& changes) {
  size_t r = 0;
  for (auto& c : changes) {
    if (c.tag == sequence_change_type::insert_tag) {
      r += c.items_to_insert->size();
    }
  }
  return r;
}

size_t sequence_changes_size_of_input(const sequence_changes_type& changes) {
  return sequence_changes_nb_to_keep(changes) + sequence_changes_nb_to_remove(changes);
}

size_t sequence_changes_size_of_result(const sequence_changes_type& changes) {
  return sequence_changes_nb_to_keep(changes) + sequence_changes_nb_to_insert(changes);
}

void sequence_changes_of_concat_tree(concat_tree_node_type* n, sequence_changes_type& destination) {
  if (n == nullptr) {
    return;
  }
  auto tag = n->get_tag();
  if (tag == concat_tree_node_type::leaf_tag) {
    assert(false); // todo
  } else if (tag == concat_tree_node_type::interior_tag) {
    assert(false); // todo
  } else if (tag == concat_tree_node_type::freeze_tag) {
    assert(false); // todo
  } else {
    assert(false);
  }
}

void apply_sequence_changes(sequence_changes_type& changes,
                            sequence_type& source,
                            sequence_type& destination) {
  assert(sequence_changes_size_of_input(changes) == source.size());
#ifndef NDEBUG
  size_t nb_result = sequence_changes_size_of_result(changes);
#endif
  for (auto& c : changes) {
    if (c.tag == sequence_change_type::keep_tag) {
      sequence_type tmp;
      source.split(c.nb_to_keep, tmp);
      tmp.swap(source);
      destination.concat(tmp);
    } else if (c.tag == sequence_change_type::remove_tag) {
      source.popn_front(c.nb_to_remove);
    } else if (c.tag == sequence_change_type::insert_tag) {
      destination.concat(*c.items_to_insert);
      delete c.items_to_insert;
      c.items_to_insert = nullptr;
    } else {
      assert(false);
    }
  }
  changes.clear();
  assert(destination.size() == nb_result);
}

#endif /*! _SACDP_TRACE_H_ */