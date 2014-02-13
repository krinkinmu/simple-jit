#include <algorithm>
#include <cassert>

#include <utils.hpp>
#include <ast.hpp>

namespace vm
{

	namespace detail
	{

		Signature::Signature(Type rtype,
								std::string name,
								Signature::ParametersType params)
			: return_type_(rtype)
			, name_(std::move(name))
			, params_(std::move(params))
		{ }

		Type Signature::return_type() const noexcept
		{ return return_type_; }

		std::string const & Signature::name() const noexcept
		{ return name_; }

		std::size_t Signature::parameters_number() const noexcept
		{ return params_.size(); }

		Signature::ParametersType::const_iterator
			Signature::begin() const noexcept
			{ return params_.cbegin(); }

		Signature::ParametersType::const_iterator
			Signature::end() const noexcept
			{ return params_.cend(); }

		Signature::ParamType const &
			Signature::at(std::size_t index) const noexcept
			{ return params_.at(index); }

		Signature::ParamType const &
			Signature::operator[](std::size_t index) const noexcept
			{ return at(index); }

	}



	Block::Block(Scope *scope)
		: scope_(scope)
	{ assert(scope_); }

	Block::~Block()
	{ std::for_each(begin(), end(), [](ASTNode * node) { delete node; }); }

	Scope * Block::scope() noexcept
	{ return scope_; }

	Scope const * Block::scope() const noexcept
	{ return scope_; }

	Block::iterator Block::begin() noexcept
	{ return instructions_.begin(); }

	Block::iterator Block::end() noexcept
	{ return instructions_.end(); }

	Block::const_iterator Block::begin() const noexcept
	{ return instructions_.cbegin(); }

	Block::const_iterator Block::end() const noexcept
	{ return instructions_.cend(); }

	std::size_t Block::size() const noexcept
	{ return instructions_.size(); }

	bool Block::empty() const noexcept
	{ return instructions_.empty(); }

	ASTNode * Block::operator[](std::size_t index) noexcept
	{ return instructions_.at(index); }

	ASTNode const * Block::operator[](std::size_t index) const noexcept
	{ return instructions_.at(index); }

	void Block::push_back(ASTNode *node)
	{
		assert(node);
		instructions_.push_back(node);
	}



	Scope::Scope(Scope *owner)
		: owner_(owner)
	{ }

	Scope::~Scope()
	{
		std::for_each(variables_begin(), variables_end(),
						[](Variable var) { delete var.definition(); } );
		std::for_each(functions_begin(), functions_end(),
						[](Function fun) { delete fun.definition(); } );
	}

	Scope::variable_iterator const
		Scope::lookup_variable(std::string const & name) noexcept
		{ return Scope::variable_iterator(variables_.find(name)); }

	Scope::const_variable_iterator const
		Scope::lookup_variable(std::string const & name) const noexcept
		{ return Scope::const_variable_iterator(variables_.find(name)); }

	Scope::function_iterator const
		Scope::lookup_function(std::string const & name) noexcept
		{ return Scope::function_iterator(functions_.find(name)); }

	Scope::const_function_iterator const
		Scope::lookup_function(std::string const & name) const noexcept
		{ return Scope::const_function_iterator(functions_.find(name)); }

	std::pair<Scope::variable_iterator, bool> const
		Scope::define_variable(Variable variable, bool replace) noexcept
		{
			auto p = variables_.insert(std::make_pair(variable.name(), variable));
			if (replace && !p.second)
				p.first->second = std::move(variable);
			return std::make_pair(Scope::variable_iterator(p.first), !p.second);
		}

	std::pair<Scope::function_iterator, bool> const
		Scope::define_function(Function function, bool replace) noexcept
		{
			auto p = functions_.insert(std::make_pair(function.name(), function));
			if (replace && !p.second)
				p.first->second = std::move(function);
			return std::make_pair(Scope::function_iterator(p.first), !p.second);
		}

	Scope * Scope::owner() noexcept
	{ return owner_; }

	Scope const * Scope::owner() const noexcept
	{ return owner_; }

	Scope::variable_iterator const Scope::variables_begin() noexcept
	{ return Scope::variable_iterator(variables_.begin()); }

	Scope::variable_iterator const Scope::variables_end() noexcept
	{ return Scope::variable_iterator(variables_.end()); }

	Scope::const_variable_iterator const Scope::variables_begin() const noexcept
	{ return Scope::const_variable_iterator(variables_.begin()); }

	Scope::const_variable_iterator const Scope::variables_end() const noexcept
	{ return Scope::const_variable_iterator(variables_.end()); }

	Scope::function_iterator const Scope::functions_begin() noexcept
	{ return Scope::function_iterator(functions_.begin()); }

	Scope::function_iterator const Scope::functions_end() noexcept
	{ return Scope::function_iterator(functions_.end()); }

	Scope::const_function_iterator const Scope::functions_begin() const noexcept
	{ return Scope::const_function_iterator(functions_.begin()); }

	Scope::const_function_iterator const Scope::functions_end() const noexcept
	{ return Scope::const_function_iterator(functions_.end()); }



	ASTNode::ASTNode(Location start, Location finish) noexcept
		: start_(std::move(start)), finish_(std::move(finish))
	{ }

	Location const & ASTNode::start() const noexcept
	{ return start_; }

	Location const & ASTNode::finish() const noexcept
	{ return finish_; }



	BinaryExprNode::BinaryExprNode(Token::Kind kind,
									ASTNode *left,
									ASTNode *right,
									Location start,
									Location finish) noexcept
		: ASTNode(std::move(start), std::move(finish))
		, kind_(kind)
		, left_(left)
		, right_(right)
	{
		static Token::Kind binaries[] = {
			Token::lor, Token::land, Token::eq, Token::neq, Token::ge, Token::le,
			Token::aor, Token::aand, Token::axor, Token::gt, Token::lt, Token::add,
			Token::sub, Token::mul, Token::div, Token::mod
		};

		assert(std::find(binaries, binaries + utils::array_size(binaries), kind) != binaries + utils::array_size(binaries));
		assert(left);
		assert(right);
	}

	BinaryExprNode::~BinaryExprNode()
	{
		delete left();
		delete right();
	}

	Token::Kind BinaryExprNode::kind() const noexcept
	{ return kind_; }

	ASTNode const * BinaryExprNode::left() const noexcept
	{ return left_; }

	ASTNode const * BinaryExprNode::right() const noexcept
	{ return right_; }

	ASTNode * BinaryExprNode::left() noexcept
	{ return left_; }

	ASTNode * BinaryExprNode::right() noexcept
	{ return right_; }



	UnaryExprNode::UnaryExprNode(Token::Kind kind,
									ASTNode *node,
									Location start,
									Location finish) noexcept
		: ASTNode(std::move(start), std::move(finish))
		, kind_(kind)
		, operand_(node)
	{
		static Token::Kind unaries[] = { Token::anot, Token::lnot, Token::sub };

		assert(std::find(unaries, unaries + utils::array_size(unaries), kind) != unaries + utils::array_size(unaries));
		assert(node);
	}

	UnaryExprNode::~UnaryExprNode()
	{ delete operand(); }

	Token::Kind UnaryExprNode::kind() const noexcept
	{ return kind_; }

	ASTNode * UnaryExprNode::operand() noexcept
	{ return operand_; }

	ASTNode const * UnaryExprNode::operand() const noexcept
	{ return operand_; }



	StringLitNode::StringLitNode(std::string value,
									Location start,
									Location finish)
		: ASTNode(std::move(start), std::move(finish))
		, value_(std::move(value))
	{ }

	std::string const & StringLitNode::value() const noexcept
	{ return value_; }



	IntLitNode::IntLitNode(std::int64_t value,
							Location start,
							Location finish) noexcept
		: ASTNode(std::move(start), std::move(finish))
		, value_(value)
	{ }

	std::int64_t IntLitNode::value() const noexcept
	{ return value_; }



	DoubleLitNode::DoubleLitNode(double value,
									Location start,
									Location finish) noexcept
		: ASTNode(std::move(start), std::move(finish))
		, value_(value)
	{ }

	double DoubleLitNode::value() const noexcept
	{ return value_; }



	LoadNode::LoadNode(Variable variable,
						Location start,
						Location finish) noexcept
		: ASTNode(std::move(start), std::move(finish))
		, variable_(std::move(variable))
	{ }

	Variable const LoadNode::variable() noexcept
	{ return variable_; }

	ConstVariable const LoadNode::variable() const noexcept
	{ return variable_; }



	StoreNode::StoreNode(Variable variable,
							ASTNode *expr,
							Token::Kind kind,
							Location start,
							Location finish) noexcept
		: ASTNode(std::move(start), std::move(finish))
		, variable_(variable)
		, kind_(kind)
		, expression_(expr)
	{
		static Token::Kind assignments[] = {
			Token::incrset, Token::decrset, Token::assign
		};

		assert(std::find(assignments, assignments + utils::array_size(assignments), kind) != assignments + utils::array_size(assignments));
		assert(expr);
	}

	StoreNode::~StoreNode()
	{ delete expression(); }

	Variable const StoreNode::variable() noexcept
	{ return variable_; }

	ConstVariable const StoreNode::variable() const noexcept
	{ return variable_; }

	ASTNode * StoreNode::expression() noexcept
	{ return expression_; }

	ASTNode const * StoreNode::expression() const noexcept
	{ return expression_; }

	Token::Kind StoreNode::kind() const noexcept
	{ return kind_; }



	NativeCallNode::NativeCallNode(detail::Signature signature,
									Location start,
									Location finish)
		: ASTNode(std::move(start), std::move(finish))
		, signature_(std::move(signature))
	{ }

	std::string const & NativeCallNode::name() const noexcept
	{ return signature_.name(); }

	Type NativeCallNode::return_type() const noexcept
	{ return signature_.return_type(); }

	std::size_t NativeCallNode::parameters_number() const noexcept
	{ return signature_.parameters_number(); }

	Type NativeCallNode::at(std::size_t index) const noexcept
	{ return signature_.at(index).first; }

	Type NativeCallNode::operator[](std::size_t index) const noexcept
	{ return at(index); }



	ForNode::ForNode(Variable variable,
						ASTNode *expr,
						Block *body,
						Location start,
						Location finish) noexcept
		: ASTNode(std::move(start), std::move(finish))
		, variable_(std::move(variable))
		, expr_(expr)
		, body_(body)
	{ }

	ForNode::~ForNode()
	{
		delete expression();
		delete body();
	}

	Variable const ForNode::variable() noexcept
	{ return variable_; }

	ConstVariable const ForNode::variable() const noexcept
	{ return variable_; }

	ASTNode * ForNode::expression() noexcept
	{ return expr_; }

	ASTNode const * ForNode::expression() const noexcept
	{ return expr_; }

	Block * ForNode::body() noexcept
	{ return body_; }

	Block const * ForNode::body() const noexcept
	{ return body_; }



	WhileNode::WhileNode(ASTNode * expr,
							Block * body,
							Location start,
							Location finish) noexcept
		: ASTNode(std::move(start), std::move(finish))
		, expr_(expr)
		, body_(body)
	{ }

	WhileNode::~WhileNode()
	{
		delete expression();
		delete body();
	}

	ASTNode * WhileNode::expression() noexcept
	{ return expr_; }

	ASTNode const * WhileNode::expression() const noexcept
	{ return expr_; }

	Block * WhileNode::body() noexcept
	{ return body_; }

	Block const * WhileNode::body() const noexcept
	{ return body_; }



	ReturnNode::ReturnNode(ASTNode *expr,
							Location start,
							Location finish) noexcept
		: ASTNode(start, finish)
		, expr_(expr)
	{ }

	ReturnNode::~ReturnNode()
	{ delete expression(); }

	ASTNode * ReturnNode::expression() noexcept
	{ return expr_; }

	ASTNode const * ReturnNode::expression() const noexcept
	{ return expr_; }



	IfNode::IfNode(ASTNode * expr,
					Block * thn,
					Block * els,
					Location start,
					Location finish) noexcept
		: ASTNode(std::move(start), std::move(finish))
		, expr_(expr)
		, thn_(thn)
		, els_(els)
	{ }

	IfNode::~IfNode()
	{
		delete expression();
		delete then_block();
		delete else_block();
	}

	ASTNode * IfNode::expression() noexcept
	{ return expr_; }

	ASTNode const * IfNode::expression() const noexcept
	{ return expr_; }

	Block * IfNode::then_block() noexcept
	{ return thn_; }

	Block const * IfNode::then_block() const noexcept
	{ return thn_; }

	Block * IfNode::else_block() noexcept
	{ return els_; }

	Block const * IfNode::else_block() const noexcept
	{ return els_; }



	CallNode::CallNode(std::string name,
						std::vector<ASTNode *> params,
						Location start,
						Location finish)
		: ASTNode(std::move(start), std::move(finish))
		, name_(std::move(name))
		, params_(std::move(params))
	{ }

	CallNode::~CallNode()
	{ std::for_each(params_.begin(), params_.end(), [](ASTNode * n) { delete n; }); }

	std::string const & CallNode::name() const noexcept
	{ return name_; }

	std::size_t CallNode::parameters_number() const noexcept
	{ return params_.size(); }

	ASTNode * CallNode::at(std::size_t index) noexcept
	{ return params_.at(index); }

	ASTNode const * CallNode::at(std::size_t index) const noexcept
	{ return params_.at(index); }

	ASTNode * CallNode::operator[](std::size_t index) noexcept
	{ return at(index); }

	ASTNode const * CallNode::operator[](std::size_t index) const noexcept
	{ return at(index); }



	PrintNode::PrintNode(std::vector<ASTNode *> params,
							Location start,
							Location finish)
		: ASTNode(std::move(start), std::move(finish))
		, params_(std::move(params))
	{ }

	PrintNode::~PrintNode()
	{ std::for_each(params_.begin(), params_.end(), [](ASTNode * n) { delete n; }); }

	std::size_t PrintNode::parameters_number() const noexcept
	{ return params_.size(); }

	ASTNode * PrintNode::at(std::size_t index) noexcept
	{ return params_.at(index); }

	ASTNode const * PrintNode::at(std::size_t index) const noexcept
	{ return params_.at(index); }

	ASTNode * PrintNode::operator[](std::size_t index) noexcept
	{ return at(index); }

	ASTNode const * PrintNode::operator[](std::size_t index) const noexcept
	{ return at(index); }

	void PrintNode::push_back(ASTNode * expr)
	{ params_.push_back(expr); }



	FunctionNode::FunctionNode(detail::Signature signature,
								Block * block,
								Location start,
								Location finish)
		: ASTNode(start, finish)
		, signature_(std::move(signature))
		, block_(block)
	{ }

	FunctionNode::~FunctionNode()
	{ delete body(); }

	std::string const & FunctionNode::name() const noexcept
	{ return signature_.name(); }

	Type FunctionNode::return_type() const noexcept
	{ return signature_.return_type(); }

	Type FunctionNode::type_at(std::size_t index) const noexcept
	{ return signature_.at(index).first; }

	std::string const & FunctionNode::name_at(std::size_t index) const noexcept
	{ return signature_.at(index).second; }

	Block * FunctionNode::body() noexcept
	{ return block_; }

	Block const * FunctionNode::body() const noexcept
	{ return block_; }

}
