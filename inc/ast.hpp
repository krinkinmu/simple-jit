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

	namespace detail
	{

		template <typename MapIt, typename ValueType>
		class BaseIterator
			: public std::iterator<
				typename std::iterator_traits<MapIt>::iterator_category,
				ValueType>
		{
			typedef std::iterator<
				typename std::iterator_traits<MapIt>::iterator_category,
				ValueType> BaseType;

		public:
			BaseIterator(MapIt iterator)
				: iterator_(iterator)
			{ }

			BaseIterator() = default;
			BaseIterator(BaseIterator const &) = default;
			BaseIterator(BaseIterator &&) noexcept = default;
			BaseIterator & operator=(BaseIterator const &) = default;
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

			ValueType & operator*() const noexcept
			{ return iterator_->second; }

			ValueType * operator->() const noexcept
			{ return &iterator_->second; }

			friend bool operator==(BaseIterator<MapIt, ValueType> const & left, BaseIterator<MapIt, ValueType> const & right)
			{ return left.iterator_ == right.iterator_; }

			friend bool operator!=(BaseIterator<MapIt, ValueType> const & left, BaseIterator<MapIt, ValueType> const & right)
			{ return !(left == right); }

		private:
			MapIt iterator_;
		};

	}

	class Signature
	{
	public:
		typedef std::pair<Type, std::string> ParamType;
		typedef std::vector<ParamType> ParametersType;

		Signature(Type rtype, std::string name, ParametersType params = ParametersType());

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



	class Block
	{
		typedef std::vector<ASTNode *> InstructionsType;

	public:
		typedef InstructionsType::const_iterator const_iterator;
		typedef InstructionsType::iterator iterator;

		Block(Scope *parent);
		virtual ~Block();

		Scope *scope() noexcept;
		Scope const *scope() const noexcept;

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

		void push_back(ASTNode * node);

	private:
		std::vector<ASTNode *> instructions_;
		Scope *scope_;
	};



	class Scope
	{
		typedef std::map<std::string, Variable *> Variables;
		typedef std::map<std::string, Function *> Functions;

	public:
		typedef detail::BaseIterator<Variables::iterator, Variable *> variable_iterator;
		typedef detail::BaseIterator<Functions::iterator, Function *> function_iterator;

		typedef detail::BaseIterator<Variables::const_iterator, Variable const *> const_variable_iterator;
		typedef detail::BaseIterator<Functions::const_iterator, Function const *> const_function_iterator;

		Scope(Scope *owner = nullptr);
		virtual ~Scope();

		variable_iterator const lookup_variable(std::string const & name) noexcept;
		const_variable_iterator const lookup_variable(std::string const & name) const noexcept;

		function_iterator const lookup_function(std::string const & name) noexcept;
		const_function_iterator const lookup_function(std::string const & name) const noexcept;

		bool define_variable(Variable *var, bool replace = false) noexcept;
		bool define_function(Function *fun, bool replace = false) noexcept;

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

	class BinaryExprNode : public ASTNode
	{
	public:
		BinaryExprNode(Token::Kind kind,
						ASTNode *left,
						ASTNode *right,
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
						ASTNode *node,
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
					ASTNode *expr,
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
		NativeCallNode(Signature signature,
						Location start = Location(),
						Location finish = Location());

		std::string const & name() const noexcept;
		Type return_type() const noexcept;

		std::size_t parameters_number() const noexcept;
		Type at(std::size_t index) const noexcept;
		Type operator[](std::size_t index) const noexcept;

	private:
		Signature signature_;
	};

	class ForNode : public ASTNode
	{
	public:
		ForNode(Variable * var,
				ASTNode *expr,
				Scope *parent,
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
		WhileNode(ASTNode * expr,
					Scope * parent,
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
		ReturnNode(ASTNode *expr = nullptr,
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
		IfNode(ASTNode * expr,
				Scope * parent,
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
					std::vector<ASTNode *> params,
					Location start = Location(),
					Location finish = Location());

		virtual ~CallNode();

		std::string const & name() const noexcept;
		std::size_t parameters_number() const noexcept;

		ASTNode * at(std::size_t index) noexcept;
		ASTNode const * at(std::size_t index) const noexcept;

		ASTNode * operator[](std::size_t index) noexcept;
		ASTNode const * operator[](std::size_t index) const noexcept;

	private:
		std::string name_;
		std::vector<ASTNode *> params_;
	};

	class PrintNode : public ASTNode
	{
	public:
		PrintNode(std::vector<ASTNode *> params,
					Location start = Location(),
					Location finish = Location());

		virtual ~PrintNode();

		std::size_t parameters_number() const noexcept;

		ASTNode * at(std::size_t index) noexcept;
		ASTNode const * at(std::size_t index) const noexcept;

		ASTNode * operator[](std::size_t index) noexcept;
		ASTNode const * operator[](std::size_t index) const noexcept;

		void push_back(ASTNode * expr);

	private:
		std::vector<ASTNode *> params_;
	};

	class Function : public LocatedInFile
	{
	public:
		Function(Signature signature,
					Scope * owner,
					Location start = Location(),
					Location finish = Location());

		virtual ~Function();

		std::string const & name() const noexcept;
		Type return_type() const noexcept;

		std::size_t parameters_number() const noexcept;
		Type type_at(std::size_t index) const noexcept;
		std::string const & name_at(std::size_t index) const noexcept;

		Block * body() noexcept;
		Block const * body() const noexcept;

		Scope * owner() noexcept;
		Scope const * owner() const noexcept;

	private:
		Signature signature_;
		Scope * params_;
		Block * block_;
	};

	class Variable : public LocatedInFile
	{
	public:
		Variable(Type type,
					std::string name,
					Scope * owner,
					Location start = Location(),
					Location finish = Location());

		std::string const & name() const noexcept;
		Type type() const noexcept;

		Scope * owner() noexcept;
		Scope const * owner() const noexcept;

	private:
		Type type_;
		std::string name_;
		Scope * owner_;
	};

}

#endif /*__AST_HPP__*/
