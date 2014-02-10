#ifndef __AST_HPP__
#define __AST_HPP__

#include <initializer_list>
#include <utility>
#include <cstddef>
#include <vector>
#include <string>

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

	class Return;
	class Store;
	class Load;
	class Call;
	class Print;
	class NativeCall;
	class UnaryExpression;
	class BinaryExpression;
	class StringLiteral;
	class IntLiteral;
	class DoubleLiteral;
	class ForLoop;
	class WhileLoop;
	class IfStatement;

	class Signature
	{
	public:
		typedef std::pair<Type, std::string> ParamType;
		typedef std::vector<ParamType> ParametersType;

		Signature(ParametersType params = ParametersType())
			: params_(std::move(params))
		{ }

		template <typename T>
		Signature(std::initializer_list<T> params)
			: params_(params)
		{ }

		Signature(Signature const &) = default;
		Signature(Signature &&) = default;

		Signature & operator=(Signature const &) = delete;
		Signature & operator=(Signature &&) = delete;

		Type return_type() const noexcept;

		size_t size() const noexcept;
		bool empty() const noexcept;

		ParametersType::const_iterator begin() const noexcept;
		ParametersType::const_iterator end() const noexcept;

		ParamType const & operator[](size_t index) const noexcept;

	private:
		ParametersType params_;
	};

	class Variable
	{
	public:
		Variable(std::string name, Type type, Scope *scope)
			: name_(std::move(name)), type_(type), scope_(scope)
		{ }

		std::string const & name() const noexcept;
		Type type() const noexcept;
		Scope * owner() noexcept;

		Variable(Variable const &) = default;
		Variable(Variable &&) = default;

		Variable & operator=(Variable const &) = delete;
		Variable & operator=(Variable &&) = delete;

	private:
		std::string name_;
		Type type_;
		Scope *owner_;
	};

	class ASTNode
	{
	public:
		ASTNode(Location start = Location(), Location finish = Location())
			: start_(std::move(start)), finish_(std::move(finish))
		{ }

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

}

#endif /*__AST_HPP__*/
