#include "json_builder.h"
#include <iostream>

using namespace std;

namespace json {

	DictItemContext Builder::StartDict() {
		if (nodes_stack_.empty() && !root_.IsNull()) {
			throw logic_error("Attempt to call method \"StartDict\" with a ready object"s);
		}
		AddNode(Dict());
		return *this;
	}

	Builder& Builder::EndDict() {
		if (nodes_stack_.empty() && !root_.IsNull()) {
			throw logic_error("Attempt to call method \"EndDict\" with a ready object"s);
		}
		if (!nodes_stack_.back()->IsDict()) {
			throw logic_error("Attempt to call method \"EndDict\" in the context of another container"s);
		}
		nodes_stack_.pop_back();
		return *this;
	}

	ArrayItemContext Builder::StartArray() {
		if (nodes_stack_.empty() && !root_.IsNull()) {
			throw logic_error("Attempt to call method \"StartArray\" with a ready object"s);
		}
		AddNode(Array());
		return *this;
	}

	Builder& Builder::EndArray() {
		if (nodes_stack_.empty() && !root_.IsNull()) {
			throw logic_error("Attempt to call method \"EndArray\" with a ready object"s);
		}
		if (!nodes_stack_.back()->IsArray()) {
			throw logic_error("Attempt to call method \"EndArray\" in the context of another container"s);
		}
		nodes_stack_.pop_back();
		return *this;
	}

	KeyItemContext Builder::Key(string&& key) {
		if (nodes_stack_.empty() && !root_.IsNull()) {
			throw logic_error("Attempt to call method \"Key\" with a ready object"s);
		}
		if (!nodes_stack_.back()->IsDict()) {
			throw logic_error("Attempt to call method \"Key\" outside of Dict"s);
		}

		Dict& dict = get<Dict>(nodes_stack_.back()->GetValue());
		nodes_stack_.push_back(&dict[move(key)]);
		return *this;
	}

	Builder& Builder::Value(Node::Value&& value) {
		if (nodes_stack_.empty() && !root_.IsNull()) {
			throw logic_error("Attempt to call method \"Value\" with a ready object"s);
		}
		AddNode(Node(move(value)));
		nodes_stack_.pop_back();
		return *this;
	}

	Node Builder::Build() {
		if (!nodes_stack_.empty()) {
			throw logic_error("The described object is not ready"s);
		}
		if (root_.IsNull()) {
			throw logic_error("Attempt to build an empty JSON"s);
		}
		return root_;
	}

	void Builder::AddNode(Node node) {
		if (nodes_stack_.empty()) {
			root_ = node;
			nodes_stack_.push_back(&root_);
		} else if (nodes_stack_.back()->IsArray()) {
			Array& arr = get<Array>(nodes_stack_.back()->GetValue());
			arr.push_back(node);
			nodes_stack_.push_back(&arr.back());
		} else {
			*nodes_stack_.back() = node;
		}
	}

	DictItemContext::DictItemContext(Builder& builder)
		: builder_(builder) {
	}
	KeyItemContext DictItemContext::Key(std::string&& key) {
		return builder_.Key(move(key));
	}
	Builder& DictItemContext::EndDict() {
		return builder_.EndDict();
	}

	KeyItemContext::KeyItemContext(Builder& builder) 
		: builder_(builder) {
	}
	DictItemContext KeyItemContext::Value(Node::Value&& value) {
		return DictItemContext(builder_.Value(move(value)));
	}
	DictItemContext KeyItemContext::StartDict() {
		return builder_.StartDict();
	}
	ArrayItemContext KeyItemContext::StartArray() {
		return builder_.StartArray();
	}

	ArrayItemContext::ArrayItemContext(Builder& builder)
		: builder_(builder) {
	}
	ArrayItemContext ArrayItemContext::Value(Node::Value&& value) {
		return ArrayItemContext(builder_.Value(move(value)));
	}
	DictItemContext ArrayItemContext::StartDict() {
		return builder_.StartDict();
	}
	ArrayItemContext ArrayItemContext::StartArray() {
		return builder_.StartArray();
	}
	Builder& ArrayItemContext::EndArray() {
		return builder_.EndArray();
	}
} // namespace json