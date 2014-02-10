#ifndef __AST_HPP__
#define __AST_HPP__

#include <initializer_list>
#include <iterator>
#include <utility>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <map>

#include <common.hpp>

namespace vm
{

	enum class Type
	{
		Invalid,
		Double,
		Int,
		String,
		Void
	};

	class Scope;
	class Visitor;

	class BinaryExprNode;
	class UnaryExprNode;
	class StringLitNode;
	class IntLitNode;
	class DoubleLitNode;
	class ReturnNode;
	class LoadNode;
	class StoreNode;
	class CallNode;
	class PrintNode;
	class NativeCallNode;
	class BlockNode;
	class ForNode;
	class WhileNode;
	class IfNode;
	class FunctionNode;
	class VariableNode;

	class Signature
	{
	public:
		typedef std::pair<Type, std::string> ParamType;
		typedef std::vector<ParamType> ParametersType;

		Signature(ParametersType params = ParametersType());

		template <typename T>
		Signature(std::initializer_list<T> params)
			: params_(params)
		{ }

		Signature(Signature const &) = default;
		Signature(Signature &&) = default;

		Signature & operator=(Signature const &) = delete;
		Signature & operator=(Signature &&) = delete;

		Type return_type() const noexcept;

		std::size_t size() const noexcept;
		bool empty() const noexcept;

		ParametersType::const_iterator begin() const noexcept;
		ParametersType::const_iterator end() const noexcept;

		ParamType const & operator[](std::size_t index) const noexcept;

	private:
		ParametersType params_;
	};

	class Variable
	{
	public:
		Variable(std::shared_ptr<VariableNode> vptr, Scope *scope);

		std::string const & name() const noexcept;
		Scope * owner() noexcept;
		std::shared_ptr<VariableNode> definition() noexcept;

	private:
		std::shared_ptr<VariableNode> def_;
		Scope *owner_;
	};

	class Function
	{
	public:
		Function(std::shared_ptr<FunctionNode> fptr, Scope *owner) noexcept;

		std::string const & name() const noexcept;
		Scope * owner() noexcept;
		std::shared_ptr<FunctionNode> definition() noexcept;

	private:
		std::shared_ptr<FunctionNode> def_;
		Scope *owner_;
	};

	namespace details
	{

		template <typename MapIt>
		class BaseIterator
			: public std::iterator<
				typename std::iterator_traits<MapIt>::iterator_category,
				typename std::iterator_traits<MapIt>::value_type::second_type>
		{
			typedef std::iterator<
				typename std::iterator_traits<MapIt>::iterator_category,
				typename std::iterator_traits<MapIt>::value_type::second_type> BaseType;

		public:
			BaseIterator() noexcept = default;
			BaseIterator(BaseIterator const &) noexcept = default;
			BaseIterator(BaseIterator &&) noexcept = default;
			BaseIterator & operator=(BaseIterator const &) noexcept = default;
			BaseIterator & operator=(BaseIterator &&) noexcept = default;

			BaseIterator & operator++() noexcept
			{
				++iterator_;
				return *this;
			}

			BaseIterator & operator--() noexcept
			{
				--iterator_;
				return *this;
			}

			BaseIterator operator++(int) const noexcept
			{
				BaseIterator iter(*this);
				++iterator_;
				return iter;
			}

			BaseIterator operator--(int) const noexcept
			{
				BaseIterator iter(*this);
				--iterator_;
				return iter;
			}

			typename BaseType::reference operator*() const noexcept
			{ return iterator->second; }

			typename BaseType::pointer operator->() const noexcept
			{ return &iterator->second; }

			friend bool operator==(BaseIterator<MapIt> const & left, BaseIterator<MapIt> const & right)
			{ return left.iterator_ == right.iterator_; }

			friend bool operator!=(BaseIterator<MapIt> const & left, BaseIterator<MapIt> const & right)
			{ return !(left == right); }

		private:
			Iterator iterator_;
		};

	}

	class Scope
	{
		typedef std::map<std::string, Variable> Variables;
		typedef std::map<std::string, Function> Functions;

	public:
		typedef details::BaseIterator<Variables::const_iterator> variables_iterator;
		typedef details::BaseIterator<Functions::const_iterator> functions_iterator;

		Scope(Scope *owner);

		variables_iterator const lookup_variable(std::string const & name) noexcept;
		functions_iterator const lookup_function(std::string const & name) noexcept;

		std::pair<variables_iterator, bool> const define_variable(Variable variable, bool replace = false);
		std::pair<functions_iterator, bool> const define_function(Function function, bool replace = false);

		Scope * owner();

		variables_iterator const variables_begin() const;
		variables_iterator const variables_end() const;
		
		functions_iterator const functions_begin() const;
		functions_iterator const functions_end() const;

	private:
		Variables variables_;
		Functions functions_;
		Scope *owner_;
	};

	class ASTNode
	{
	public:
		ASTNode(Location start = Location(),
				Location finish = Location()) noexcept;

		ASTNode(ASTNode const &) = delete;
		ASTNode(ASTNode &&) = delete;
		ASTNode & operator=(ASTNode const &) = delete;
		ASTNode & operator=(ASTNode &&) = delete;

		virtual ~ASTNode()
		{ }

		virtual void visit(ASTVisitor & visitor) = 0;
		virtual void visit_children(ASTVisitor & visitor) = 0;

		Location const & start() const noexcept;
		Location const & finish() const noexcept;

	private:
		Location start_;
		Location finish_;
	};

	class BinaryExprNode : public ASTNode
	{
	public:
		BinaryExprNode(Token::Kind kind,
						ASTNode *left,
						ASTNode *right,
						Location start = Location(),
						Location finish = Location()) noexcept;

		Token::Kind kind() const noexcept;
		ASTNode * left() const noexcept;
		ASTNode * right() const noexcept;

		virtual void visit(Visitor & visitor);
		virtual void visit_children(Visitor & visitor);
	private:
		Token::Kind kind_;
		ASTNode *left_;
		ASTNode *right_;
	};

	class UnaryExprNode : public ASTNode
	{
	public:
		UnaryExprNode(Token::Kind kind,
						ASTNode * node,
						Location start = Location(),
						Location finish = Location()) noexcept;

		Token::Kind kind() const noexcept;
		ASTNode *operand() const noexcept;

		virtual void visit(Visitor & visitor);
		virtual void visit_children(Visitor & visitor);
	private:
		Token::Kind kind_;
		ASTNode *operand_;
	};

	class StringLitNode : public ASTNode
	{
	public:
		StringLitNode(std::string value,
						Location start = Location(),
						Location finish = Location());

		std::string const & value() const noexcept;

		virtual void visit(Visitor & visitor);
		virtual void visit_children(Visitor & visitor);

	private:
		std::string value_;
	};

	class IntLitNode : public ASTNode
	{
	public:
		IntLitNode(std::intmax_t value,
					Location start = Location(),
					Location finish = Location());

		std::intmax_t value() const noexcept;

		virtual void visit(Visitor & visitor);
		virtual void visit_children(Visitor & visitor);

	private:
		std::intmax_t value_;
	};

	class DoubleLitNode : public ASTNode
	{
	public:
		DoubleLitNode(double value,
						Location start = Location(),
						Location finsih = Location());

		double value() const noexcept;

		virtual void visit(Visitor & visitor);
		virtual void visit_children(Visitor & visitor);

	private:
		double value_;
	};

}

#endif /*__AST_HPP__*/
