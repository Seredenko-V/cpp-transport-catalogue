#pragma once

#include "json.h"
#include <string>
#include <vector>
#include <optional>
#include <stdexcept>

namespace json {
	class DictItemContext;
	class KeyItemContext;
	class ArrayItemContext;

	class Builder {
	public:
		KeyItemContext Key(std::string&& key);
		Builder& Value(Node::Value&& value);
		DictItemContext StartDict();
		Builder& EndDict();
		ArrayItemContext StartArray();
		Builder& EndArray();

		Node Build();

	private:
		Node root_;
		std::vector<Node*> nodes_stack_;
		void AddNode(Node node);
	};

	class DictItemContext {
	public:
		DictItemContext(Builder& builder);
		KeyItemContext Key(std::string&& key);
		Builder& EndDict();

	private:
		Builder& builder_;
	};

	class KeyItemContext {
	public:
		KeyItemContext(Builder& builder);
		DictItemContext Value(Node::Value&& value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();

	private:
		Builder& builder_;
	};

	class ArrayItemContext {
	public:
		ArrayItemContext(Builder& builder);
		ArrayItemContext Value(Node::Value&& value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndArray();

	private:
		Builder& builder_;
	};

} // namespace json