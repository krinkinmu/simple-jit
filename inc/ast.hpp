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

	class BinaryExprNode;
	class UnaryExprNode;
	class StringLitNode;
	class IntLitNode;
	class DoubleLitNode;
	class LoadNode;
	class StoreNode;
	class ReturnNode;
	class NativeCallNode;
	class CallNode;
	class PrintNode;
	class ForNode;
	class WhileNode;
	class IfNode;
	class FunctionNode;
	class VariableNode;

	namespace detail
	{

		template <typename MapIt, typename ValueType>
		class BaseIterator
			: public std::iterator<
				typename std::iterator_traits<MapIt>::iterator_category,
				typename ValueType>
		{
			typedef std::iterator<
				typename std::iterator_traits<MapIt>::iterator_category,
				typename ValueType> BaseType;

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

			ValueType & operator*() const noexcept
			{ return iterator->second; }

			ValueType * operator->() const noexcept
			{ return &iterator->second; }

			friend bool operator==(BaseIterator<MapIt> const & left, BaseIterator<MapIt> const & right)
			{ return left.iterator_ == right.iterator_; }

			friend bool operator!=(BaseIterator<MapIt> const & left, BaseIterator<MapIt> const & right)
			{ return !(left == right); }

		private:
			Iterator iterator_;
		};

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

		template <typename DefinitionType, typename ScopeType>
		class ScopedDefinition
		{
		public:
			ScopedDefinition(DefinitionType *def, ScopeType *scope)
				: def_(def), scope_(scope)
			{ }

			ScopedDefinition(ScopedDefinition const &) noexcept = default;
			ScopedDefinition(ScopedDefinition &&) noexcept = default;
			ScopedDefinition & operator=(ScopdedDefinition const &) noexcept = default;
			ScopdeDefinition & operator=(ScopdeDefinition &&) noexcept = default;

			template <typename DefTy, typename ScTy>
			ScopedDefinition(ScopedDefinition<DefTy, ScTy> const & other) noexcept
				: def_(other.definition()), scope_(other.owner())
			{ }

			template <typename DefTy, typename ScTy>
			ScopdeDefinition(ScopedDefinition<DefTy, ScTy> && other) noexcept
				: def_(other.definition()), scope_(other.owner())
			{ }

			template <typename DefTy, typename ScTy>
			ScopedDefinition & operator=(ScopdeDefinition<DefTy, ScTy> const & other) noexcept
			{
				def_ = other.definition();
				scope_ = other.owner();
				return *this;
			}

			template <typename DefTy, typename ScTy>
			ScopedDefinition & operator=(ScopedDefinition<DefTy, ScTy> && other) noexcept
			{
				def_ = other.definition();
				scope_ = other.owner();
				return *this;
			}

			std::string const & name() const noexcept
			{ return def_->name(); }

			typename std::remove_const<ScopeType>::type const *
					owner() const noexcept
			{ return owner_; }

			typename std::remove_const<ScopeType>::type *
					owner() noexcept
			{ return owner_; }

			typename std::remove_const<DefinitionType>::type const *
					definition() const noexcept
			{ return def_; }

			typename std::remove_const<DefinitionType>::type *
					definition() noexcept
			{ return def_; }

		private:
			DefinitionType *def_;
			ScopeType *scope_;
		};

		typedef ScopedDefinition<VariableNode, Scope> Variable;
		typedef ScopedDefinition<FunctionNode, Scope> Function;

		typedef ScopedDefinition<VariableNode const, Scope const> ConstVariable;
		typedef ScopedDefinition<FunctionNode const, Scope const> ConstFunction;

	}

	class Block
	{
	typedef std::vector<ASTNode *> InstructionsType;
	public:
		typedef InstructionsType::const_iterator const_iterator;
		typedef InstructionsType::iterator iterator;

		Block(Scope *scope);
		virtual ~Block();

		Scope *scope() noexcept;
		Scope const *scope() const noexcept;

		iterator begin() noexcept;
		iterator end() noexcept;

		const_iterator begin() const noexcept;
		const_iterator end() const noexcept;

		std::size_t size() const noexcept;
		bool empty() const noexcept;

		ASTNode * operator[](std::size_t index) noexcept;
		ASTNode const * operator[](std::size_t index) const noexcept;

		void push_back(ASTNode * node);

	private:
		std::vector<ASTNode *> instructions_;
		Scope *scope_;
	};

	class Scope
	{
		typedef std::map<std::string, detail::Variable> Variables;
		typedef std::map<std::string, detail::Function> Functions;

	public:
		typedef detail::BaseIterator<Variables::iterator, Variable> variable_iterator;
		typedef detail::BaseIterator<Functions::iterator, Function> function_iterator;

		typedef detail::BaseIterator<Variables::iterator, Variable const> const_variable_iterator;
		typedef detail::BaseIterator<Functions::iterator, Function const> const_function_iterator;

		Scope(Scope *owner);
		virtual ~Scope();

		variable_iterator const lookup_variable(std::string const & name) noexcept;
		const_variable_iterator const lookup_variable(std::string const & name) const noexcept;

		function_iterator const lookup_function(std::string const & name) noexcept;
		const_function_iterator const lookup_function(std::string const & name) const noexcept;

		std::pair<variable_iterator, bool> const define_variable(Variable variable, bool replace = false);
		std::pair<function_iterator, bool> const define_function(Function function, bool replace = false);

		Scope * owner();
		Scope const * owner() const;

		variable_iterator const variables_begin();
		variable_iterator const variables_end();

		const_variable_iterator const variables_begin() const;
		const_variable_iterator const variables_end() const;
		
		function_iterator const functions_begin();
		function_iterator const functions_end();

		const_function_iterator const functions_begin() const;
		const_function_iterator const functions_end() const;

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

		virtual ~ASTNode() { }

		virtual void visit(ASTVisitor & visitor) { }
		virtual void visit_children(ASTVisitor & visitor) { }

		virtual void visit(ASTVisitor const & visitor) { }
		virtual void visit_children(ASTVisitor const & visitor) { }

		virtual void visit(ASTVisitor & visitor) const { }
		virtual void visit_children(ASTVisitor & visitor) const { }

		virtual void visit(ASTVisitor const & visitor) const { }
		virtual void visit_children(ASTVisitor const & visitor) const { }

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
		IntLitNode(std::intmax_t value,
					Location start = Location(),
					Location finish = Location()) noexcept;

		std::intmax_t value() const noexcept;

	private:
		std::intmax_t value_;
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
		LoadNode(detail::Variable variable,
					Location start = Location(),
					Location finish = Location()) noexcept;

		detail::Variable variable() noexcept;
		detail::ConstVariable variable() const noexcept;

	private:
		detail::Variable variable_;
	};

	class StoreNode : public ASTNode
	{
	public:
		StoreNode(detail::Variable variable,
					ASTNode *expr,
					Token::Kind kind,
					Location start = Location(),
					Location finish = Location()) noexcept;

		virtual ~StoreNode();

		detail::Variable variable() noexcept;
		detail::ConstVariable variable() const noexcept;

		ASTNode * expression() noexcept;
		ASTNode const * expression() const noexcept;

		Token::Kind kind() const noexcept;

	pivate:
		detail::Variable variable_;
		ASTNode *expression_;
		Token::Kind kind_;
	};

	class NativeCallNode : public ASTNode
	{
	public:
		NativeCallNode(detail::Signature signature,
						Location start = Location(),
						Location finish = Location());

		std::string const & name() const noexcept;
		Type return_type() const noexcept;

		std::size_t parameters_number() const noexcept;
		Type type_at(std::size_t index) const noexcept;
		Type operator[](std::size_t index) const noexcept;

	private:
		detail::Signature signature_;
	};

	class ForNode : public ASTNode
	{
	public:
		ForNode(Variable variable,
				ASTNode *expr,
				Block *body,
				Location start = Location(),
				Location finish = Location());

		virtual ~ForNode();

		Variable variable() noexcept;
		ConstVariable variable() const noexcept;

		ASTNode * expression() noexcept;
		ASTNode const * expression() const noexcept;

		Block * body() noexcept;
		Block const * body() const noexcept;

	private:
		Variable variable_;
		ASTNode * expr_;
		Block *body_;
	};

	class WhileNode : public ASTNode
	{
	public:
		WhileNode(ASTNode * expr,
					Block * body,
					Location start = Location(),
					Location finish = Location());

		virtual ~WhileNode();

		ASTNode * expression() noexcept;
		ASTNode const * expression() const noexcept;

		Block * body() noexcept;
		Block const * body() const noexcept;

	private:
		ASTNode * expression_;
		Block * body_;
	};

}

#endif /*__AST_HPP__*/
