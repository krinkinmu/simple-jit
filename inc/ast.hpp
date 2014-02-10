#ifndef __AST_HPP__
#define __AST_HPP__

#include <initializer_list>
#include <utility>
#include <cstddef>
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

	class ReturnNode;
	class LoadNode;
	class StoreNode;
	class CallNode;
	class PrintNode;
	class NativeCallNode;
	class UnaryExprNode;
	class BinaryExprNode;
	class StringLitNode;
	class IntLitNode;
	class DoubleLitNode;
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

	class Scope
	{
	public:
		Scope(Scope *owner);

		Variable const lookup_variable(std::string const & name) noexcept;
		Function const lookup_function(std::string const & name) noexcept;

		bool define_variable(Variable variable, bool replace = false);
		bool define_function(Function function, bool replace = false);

		Scope * owner();

	private:
		std::map<std::string, Variable> variables_;
		std::map<std::string, Function> functions_;
		Scope *owner_;
	};

	class ASTNode
	{
	public:
		ASTNode(Location start = Location(), Location finish = Location());

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
