#ifndef __AST_HPP__
#define __AST_HPP__

#include <initializer_list>
#include <type_traits>
#include <iterator>
#include <utility>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include <memory>

#include <common.hpp>
#include <token.hpp>

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
	class Block;
	class Visitor;

	class ASTNode;

	class BinaryExprNode;
	class UnaryExprNode;
	class StringLitNode;
	class IntLitNode;
	class DoubleLitNode;
	class LoadNode;
	class StoreNode;
	class ForNode;
	class WhileNode;
	class NativeCallNode;
	class ReturnNode;
	class IfNode;
	class CallNode;
	class PrintNode;

	class Function;
	class Variable;


	class Signature
	{
	public:
		typedef std::pair<Type, std::string> ParamType;
		typedef std::vector<ParamType> ParametersType;

		Signature(Type rtype, std::string name);

		Signature(Signature const &) = default;
		Signature(Signature &&) = default;

		Signature & operator=(Signature const &) = delete;
		Signature & operator=(Signature &&) = delete;

		Type return_type() const noexcept;
		std::string const & name() const noexcept;
		std::size_t parameters_number() const noexcept;

		ParametersType::const_iterator begin() const noexcept;
		ParametersType::const_iterator end() const noexcept;

		ParamType const & at(std::size_t index) const noexcept;
		ParamType const & operator[](std::size_t index) const noexcept;

		void push_back(ParamType param);

	private:
		Type return_type_;
		std::string name_;
		ParametersType params_;
	};

	class LocatedInFile
	{
	public:
		LocatedInFile(Location start = Location(), Location finish = Location()) noexcept;

		Location const & start() const noexcept;
		Location const & finish() const noexcept;

		void set_start(Location start) noexcept;
		void set_finish(Location finish) noexcept;

	private:
		Location start_;
		Location finish_;
	};

	class Scope
	{
		typedef std::map<std::string, Variable *> Variables;
		typedef std::map<std::string, Function *> Functions;

	public:
		typedef Variables::iterator variable_iterator;
		typedef Functions::iterator function_iterator;

		typedef Variables::const_iterator const_variable_iterator;
		typedef Functions::const_iterator const_function_iterator;

		Scope(Scope *owner = nullptr);
		virtual ~Scope();

		Variable * lookup_variable(std::string const & name) noexcept;
		Variable const * lookup_variable(std::string const & name) const noexcept;

		Function * lookup_function(std::string const & name) noexcept;
		Function const * lookup_function(std::string const & name) const noexcept;

		void define_variable(std::unique_ptr<Variable> var);
		void define_function(std::unique_ptr<Function> fun);

		Scope * owner() noexcept;
		Scope const * owner() const noexcept;

		variable_iterator const variables_begin() noexcept;
		variable_iterator const variables_end() noexcept;

		const_variable_iterator const variables_begin() const noexcept;
		const_variable_iterator const variables_end() const noexcept;
		
		function_iterator const functions_begin() noexcept;
		function_iterator const functions_end() noexcept;

		const_function_iterator const functions_begin() const noexcept;
		const_function_iterator const functions_end() const noexcept;

	private:
		Variables variables_;
		Functions functions_;
		Scope *owner_;
		std::vector<Scope const *> children_;

		void register_child(Scope const * child);
	};

	class ASTNode : public LocatedInFile
	{
	public:
		ASTNode(Location start = Location(),
				Location finish = Location()) noexcept;

		ASTNode(ASTNode const &) = delete;
		ASTNode(ASTNode &&) = delete;
		ASTNode & operator=(ASTNode const &) = delete;
		ASTNode & operator=(ASTNode &&) = delete;

		virtual ~ASTNode() { }

		virtual void visit(Visitor &) { }
		virtual void visit_children(Visitor &) { }

		virtual void visit(Visitor const &) { }
		virtual void visit_children(Visitor const &) { }

		virtual void visit(Visitor &) const { }
		virtual void visit_children(Visitor &) const { }

		virtual void visit(Visitor const &) const { }
		virtual void visit_children(Visitor const &) const { }
	};

	class Block : public ASTNode
	{
		typedef std::vector<ASTNode *> InstructionsType;

	public:
		typedef InstructionsType::const_iterator const_iterator;
		typedef InstructionsType::iterator iterator;

		Block(Scope *inner,
				Location start = Location(),
				Location finish = Location());
		virtual ~Block();

		Scope *scope() noexcept;
		Scope const *scope() const noexcept;

		Scope *owner() noexcept;
		Scope const *owner() const noexcept;

		iterator begin() noexcept;
		iterator end() noexcept;

		const_iterator begin() const noexcept;
		const_iterator end() const noexcept;

		std::size_t size() const noexcept;
		bool empty() const noexcept;

		ASTNode * at(std::size_t index) noexcept;
		ASTNode const * at(std::size_t index) const noexcept;

		ASTNode * operator[](std::size_t index) noexcept;
		ASTNode const * operator[](std::size_t index) const noexcept;

		void push_back(std::unique_ptr<ASTNode> node);

	private:
		std::vector<ASTNode *> instructions_;
		Scope *inner_;
	};

	class BinaryExprNode : public ASTNode
	{
	public:
		BinaryExprNode(Token::Kind kind,
						std::unique_ptr<ASTNode> left,
						std::unique_ptr<ASTNode> right,
						Location start = Location(),
						Location finish = Location()) noexcept;

		virtual ~BinaryExprNode();

		Token::Kind kind() const noexcept;

		ASTNode const * left() const noexcept;
		ASTNode const * right() const noexcept;

		ASTNode * left() noexcept;
		ASTNode * right() noexcept;

	private:
		Token::Kind kind_;
		ASTNode *left_;
		ASTNode *right_;
	};

	class UnaryExprNode : public ASTNode
	{
	public:
		UnaryExprNode(Token::Kind kind,
						std::unique_ptr<ASTNode> node,
						Location start = Location(),
						Location finish = Location()) noexcept;
		virtual ~UnaryExprNode();

		Token::Kind kind() const noexcept;
		ASTNode *operand() noexcept;
		ASTNode const * operand() const noexcept;

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

	private:
		std::string value_;
	};

	class IntLitNode : public ASTNode
	{
	public:
		IntLitNode(std::int64_t value,
					Location start = Location(),
					Location finish = Location()) noexcept;

		std::int64_t value() const noexcept;

	private:
		std::int64_t value_;
	};

	class DoubleLitNode : public ASTNode
	{
	public:
		DoubleLitNode(double value,
						Location start = Location(),
						Location finsih = Location()) noexcept;

		double value() const noexcept;

	private:
		double value_;
	};

	class LoadNode : public ASTNode
	{
	public:
		LoadNode(Variable *var,
					Location start = Location(),
					Location finish = Location()) noexcept;

		Variable * variable() noexcept;
		Variable const * variable() const noexcept;

	private:
		Variable * variable_;
	};

	class StoreNode : public ASTNode
	{
	public:
		StoreNode(Variable * var,
					std::unique_ptr<ASTNode> expr,
					Token::Kind kind,
					Location start = Location(),
					Location finish = Location()) noexcept;

		virtual ~StoreNode();

		Variable * variable() noexcept;
		Variable const * variable() const noexcept;

		ASTNode * expression() noexcept;
		ASTNode const * expression() const noexcept;

		Token::Kind kind() const noexcept;

	private:
		Variable * variable_;
		Token::Kind kind_;
		ASTNode *expression_;
	};

	class NativeCallNode : public ASTNode
	{
	public:
		NativeCallNode(std::unique_ptr<Signature> signature,
						Location start = Location(),
						Location finish = Location()) noexcept;
		virtual ~NativeCallNode();

		std::string const & name() const noexcept;
		Type return_type() const noexcept;

		std::size_t parameters_number() const noexcept;
		Type at(std::size_t index) const noexcept;
		Type operator[](std::size_t index) const noexcept;

	private:
		Signature *signature_;
	};

	class ForNode : public ASTNode
	{
	public:
		ForNode(Variable * var,
				std::unique_ptr<ASTNode> expr,
				std::unique_ptr<Block> body,
				Location start = Location(),
				Location finish = Location()) noexcept;

		virtual ~ForNode();

		Variable * variable() noexcept;
		Variable const * variable() const noexcept;

		ASTNode * expression() noexcept;
		ASTNode const * expression() const noexcept;

		Block * body() noexcept;
		Block const * body() const noexcept;

	private:
		Variable *variable_;
		ASTNode * expr_;
		Block *body_;
	};

	class WhileNode : public ASTNode
	{
	public:
		WhileNode(std::unique_ptr<ASTNode> expr,
					std::unique_ptr<Block> body,
					Location start = Location(),
					Location finish = Location()) noexcept;

		virtual ~WhileNode();

		ASTNode * expression() noexcept;
		ASTNode const * expression() const noexcept;

		Block * body() noexcept;
		Block const * body() const noexcept;

	private:
		ASTNode * expr_;
		Block * body_;
	};

	class ReturnNode : public ASTNode
	{
	public:
		ReturnNode(std::unique_ptr<ASTNode> expr = std::unique_ptr<ASTNode>(),
					Location start = Location(),
					Location finish = Location()) noexcept;

		virtual ~ReturnNode();

		ASTNode * expression() noexcept;
		ASTNode const * expression() const noexcept;

	private:
		ASTNode *expr_;
	};

	class IfNode : public ASTNode
	{
	public:
		IfNode(std::unique_ptr<ASTNode> expr,
				std::unique_ptr<Block> if_true,
				std::unique_ptr<Block> if_false = std::unique_ptr<Block>(),
				Location start = Location(),
				Location finish = Location()) noexcept;

		virtual ~IfNode();

		ASTNode * expression() noexcept;
		ASTNode const * expression() const noexcept;

		Block * then_block() noexcept;
		Block const * then_block() const noexcept;

		Block * else_block() noexcept;
		Block const * else_block() const noexcept;

	private:
		ASTNode *expr_;
		Block *thn_;
		Block *els_;
	};

	class CallNode : public ASTNode
	{
	public:
		CallNode(std::string name,
					Location start = Location(),
					Location finish = Location());

		virtual ~CallNode();

		std::string const & name() const noexcept;
		std::size_t parameters_number() const noexcept;

		ASTNode * at(std::size_t index) noexcept;
		ASTNode const * at(std::size_t index) const noexcept;

		ASTNode * operator[](std::size_t index) noexcept;
		ASTNode const * operator[](std::size_t index) const noexcept;

		void push_back(std::unique_ptr<ASTNode> arg);

	private:
		std::string name_;
		std::vector<ASTNode *> params_;
	};

	class PrintNode : public ASTNode
	{
	public:
		PrintNode(Location start = Location(),
					Location finish = Location()) noexcept;

		virtual ~PrintNode();

		std::size_t parameters_number() const noexcept;

		ASTNode * at(std::size_t index) noexcept;
		ASTNode const * at(std::size_t index) const noexcept;

		ASTNode * operator[](std::size_t index) noexcept;
		ASTNode const * operator[](std::size_t index) const noexcept;

		void push_back(std::unique_ptr<ASTNode> expr);

	private:
		std::vector<ASTNode *> params_;
	};

	class Function : public LocatedInFile
	{
	public:
		Function(std::unique_ptr<Signature> signature,
					std::unique_ptr<Block> body,
					Location start = Location(),
					Location finish = Location()) noexcept;

		virtual ~Function();

		std::string const & name() const noexcept;
		Type return_type() const noexcept;

		std::size_t parameters_number() const noexcept;
		Type type_at(std::size_t index) const noexcept;
		std::string const & name_at(std::size_t index) const noexcept;

		Block * body() noexcept;
		Block const * body() const noexcept;

	private:
		Signature * signature_;
		Block * body_;
	};

	class Variable : public LocatedInFile
	{
	public:
		Variable(Type type,
					std::string name,
					Location start = Location(),
					Location finish = Location());

		std::string const & name() const noexcept;
		Type type() const noexcept;

		Scope * owner() noexcept;
		Scope const * owner() const noexcept;

		void set_owner(Scope * scope) noexcept;

	private:
		Type type_;
		std::string name_;
		Scope * owner_;
	};

}

#endif /*__AST_HPP__*/
