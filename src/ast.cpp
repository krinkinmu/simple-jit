#include <algorithm>
#include <cassert>

#include <utils.hpp>
#include <ast.hpp>

namespace vm
{


	Signature::Signature(Type rtype, std::string name)
		: return_type_(rtype)
		, name_(std::move(name))
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

	void Signature::push_back(Signature::ParamType param)
	{ params_.push_back(std::move(param)); }



	Block::Block(Scope *inner, Location start, Location finish)
		: ASTNode(std::move(start), std::move(finish))
		, inner_(inner)
	{ assert(inner_); }

	Block::~Block()
	{ std::for_each(begin(), end(), [](ASTNode * node) { delete node; }); }

	Scope * Block::scope() noexcept
	{ return inner_; }

	Scope const * Block::scope() const noexcept
	{ return inner_; }

	Scope * Block::owner() noexcept
	{ return scope()->owner(); }

	Scope const * Block::owner() const noexcept
	{ return scope()->owner(); }

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

	ASTNode * Block::at(std::size_t index) noexcept
	{ return instructions_.at(index); }

	ASTNode const * Block::at(std::size_t index) const noexcept
	{ return instructions_.at(index); }

	ASTNode * Block::operator[](std::size_t index) noexcept
	{ return at(index); }

	ASTNode const * Block::operator[](std::size_t index) const noexcept
	{ return at(index); }

	void Block::push_back(std::unique_ptr<ASTNode> node)
	{
		instructions_.push_back(node.get());
		node.release();
	}



	Scope::Scope(Scope *owner)
		: owner_(owner)
	{
		if (this->owner())
			this->owner()->register_child(this);
	}

	Scope::~Scope()
	{
		typedef std::pair<std::string const, Variable *> VarPair;
		typedef std::pair<std::string const, Function *> FunPair;

		std::for_each(variables_begin(), variables_end(),
						[](VarPair const & var) { delete var.second; });
		std::for_each(functions_begin(), functions_end(),
						[](FunPair const & fun) { delete fun.second; });
		std::for_each(children_.begin(), children_.end(),
						[](Scope const * scope) { delete scope; });
	}

	Variable * Scope::lookup_variable(std::string const & name) noexcept
	{
		return const_cast<Variable *>(const_cast<Scope const *>(this)->lookup_variable(name));
	}

	Variable const * Scope::lookup_variable(std::string const & name) const noexcept
	{
		Scope::const_variable_iterator it(variables_.find(name));
		if (it != variables_.cend())
			return it->second;

		if (owner() != nullptr)
			return owner()->lookup_variable(name);

		return nullptr;
	}

	Function * Scope::lookup_function(std::string const & name) noexcept
	{ return const_cast<Function *>(const_cast<Scope const *>(this)->lookup_function(name)); }

	Function const * Scope::lookup_function(std::string const & name) const noexcept
	{
		Scope::const_function_iterator it(functions_.find(name));
		if (it != functions_.cend())
			return it->second;

		if (owner() != nullptr)
			return owner()->lookup_function(name);

		return nullptr;
	}

	void Scope::define_variable(std::unique_ptr<Variable> var)
	{
		variables_[var->name()] = var.get();
		var->set_owner(this);
		var.release();
	}

	void Scope::define_function(std::unique_ptr<Function> fun)
	{
		functions_[fun->name()] = fun.get();
		fun.release();
	}

	Scope * Scope::owner() noexcept
	{ return owner_; }

	Scope const * Scope::owner() const noexcept
	{ return owner_; }

	Scope::variable_iterator const Scope::variables_begin() noexcept
	{ return variables_.begin(); }

	Scope::variable_iterator const Scope::variables_end() noexcept
	{ return variables_.end(); }

	Scope::const_variable_iterator const Scope::variables_begin() const noexcept
	{ return variables_.begin(); }

	Scope::const_variable_iterator const Scope::variables_end() const noexcept
	{ return variables_.end(); }

	Scope::function_iterator const Scope::functions_begin() noexcept
	{ return functions_.begin(); }

	Scope::function_iterator const Scope::functions_end() noexcept
	{ return functions_.end(); }

	Scope::const_function_iterator const Scope::functions_begin() const noexcept
	{ return functions_.begin(); }

	Scope::const_function_iterator const Scope::functions_end() const noexcept
	{ return functions_.end(); }

	void Scope::register_child(Scope const * child)
	{ children_.push_back(child); }



	LocatedInFile::LocatedInFile(Location start, Location finish) noexcept
		: start_(std::move(start)), finish_(std::move(finish))
	{ }

	Location const & LocatedInFile::start() const noexcept
	{ return start_; }

	Location const & LocatedInFile::finish() const noexcept
	{ return finish_; }

	void LocatedInFile::set_start(Location start) noexcept
	{ start_ = std::move(start); }

	void LocatedInFile::set_finish(Location finish) noexcept
	{ finish_ = std::move(finish); }



	ASTNode::ASTNode(Location start, Location finish) noexcept
		: LocatedInFile(std::move(start), std::move(finish))
	{ }



	BinaryExprNode::BinaryExprNode(Token::Kind kind,
									std::unique_ptr<ASTNode> left,
									std::unique_ptr<ASTNode> right,
									Location start,
									Location finish) noexcept
		: ASTNode(std::move(start), std::move(finish))
		, kind_(kind)
		, left_(left.release())
		, right_(right.release())
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
									std::unique_ptr<ASTNode> node,
									Location start,
									Location finish) noexcept
		: ASTNode(std::move(start), std::move(finish))
		, kind_(kind)
		, operand_(node.release())
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



	LoadNode::LoadNode(Variable *var,
						Location start,
						Location finish) noexcept
		: ASTNode(std::move(start), std::move(finish))
		, variable_(var)
	{ assert(var); }

	Variable * LoadNode::variable() noexcept
	{ return variable_; }

	Variable const * LoadNode::variable() const noexcept
	{ return variable_; }



	StoreNode::StoreNode(Variable *var,
							std::unique_ptr<ASTNode> expr,
							Token::Kind kind,
							Location start,
							Location finish) noexcept
		: ASTNode(std::move(start), std::move(finish))
		, variable_(var)
		, kind_(kind)
		, expression_(expr.release())
	{
		static Token::Kind assignments[] = {
			Token::incrset, Token::decrset, Token::assign
		};

		assert(std::find(assignments, assignments + utils::array_size(assignments), kind) != assignments + utils::array_size(assignments));
		assert(expr);
		assert(var);
	}

	StoreNode::~StoreNode()
	{ delete expression(); }

	Variable * StoreNode::variable() noexcept
	{ return variable_; }

	Variable const * StoreNode::variable() const noexcept
	{ return variable_; }

	ASTNode * StoreNode::expression() noexcept
	{ return expression_; }

	ASTNode const * StoreNode::expression() const noexcept
	{ return expression_; }

	Token::Kind StoreNode::kind() const noexcept
	{ return kind_; }



	NativeCallNode::NativeCallNode(std::unique_ptr<Signature> signature,
									Location start,
									Location finish) noexcept
		: ASTNode(std::move(start), std::move(finish))
		, signature_(signature.release())
	{ assert(signature_); }

	NativeCallNode::~NativeCallNode()
	{ delete signature_; }

	std::string const & NativeCallNode::name() const noexcept
	{ return signature_->name(); }

	Type NativeCallNode::return_type() const noexcept
	{ return signature_->return_type(); }

	std::size_t NativeCallNode::parameters_number() const noexcept
	{ return signature_->parameters_number(); }

	Type NativeCallNode::at(std::size_t index) const noexcept
	{ return signature_->at(index).first; }

	Type NativeCallNode::operator[](std::size_t index) const noexcept
	{ return at(index); }



	ForNode::ForNode(Variable *var,
						std::unique_ptr<ASTNode> expr,
						std::unique_ptr<Block> body,
						Location start,
						Location finish) noexcept
		: ASTNode(std::move(start), std::move(finish))
		, variable_(var)
		, expr_(expr.release())
		, body_(body.release())
	{
		assert(variable_);
		assert(expr_);
		assert(body_);
	}

	ForNode::~ForNode()
	{
		delete expression();
		delete body();
	}

	Variable * ForNode::variable() noexcept
	{ return variable_; }

	Variable const * ForNode::variable() const noexcept
	{ return variable_; }

	ASTNode * ForNode::expression() noexcept
	{ return expr_; }

	ASTNode const * ForNode::expression() const noexcept
	{ return expr_; }

	Block * ForNode::body() noexcept
	{ return body_; }

	Block const * ForNode::body() const noexcept
	{ return body_; }



	WhileNode::WhileNode(std::unique_ptr<ASTNode> expr,
							std::unique_ptr<Block> body,
							Location start,
							Location finish) noexcept
		: ASTNode(std::move(start), std::move(finish))
		, expr_(expr.release())
		, body_(body.release())
	{
		assert(expr_);
		assert(body_);
	}

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



	ReturnNode::ReturnNode(std::unique_ptr<ASTNode> expr,
							Location start,
							Location finish) noexcept
		: ASTNode(start, finish)
		, expr_(expr.release())
	{ }

	ReturnNode::~ReturnNode()
	{ delete expression(); }

	ASTNode * ReturnNode::expression() noexcept
	{ return expr_; }

	ASTNode const * ReturnNode::expression() const noexcept
	{ return expr_; }



	IfNode::IfNode(std::unique_ptr<ASTNode> expr,
					std::unique_ptr<Block> if_true,
					std::unique_ptr<Block> if_false,
					Location start,
					Location finish) noexcept
		: ASTNode(std::move(start), std::move(finish))
		, expr_(expr.release())
		, thn_(if_true.release())
		, els_(if_false.release())
	{
		assert(expr_);
		assert(thn_);
		assert(els_);
	}

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
						Location start,
						Location finish)
		: ASTNode(std::move(start), std::move(finish))
		, name_(std::move(name))
	{ }

	CallNode::~CallNode()
	{
		std::for_each(params_.begin(), params_.end(),
				[](ASTNode * n) { delete n; });
	}

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

	void CallNode::push_back(std::unique_ptr<ASTNode> arg)
	{
		params_.push_back(arg.get());
		arg.release();
	}



	PrintNode::PrintNode(Location start,
							Location finish) noexcept
		: ASTNode(std::move(start), std::move(finish))
	{ }

	PrintNode::~PrintNode()
	{
		std::for_each(params_.begin(), params_.end(),
				[](ASTNode * n) { delete n; });
	}

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

	void PrintNode::push_back(std::unique_ptr<ASTNode> expr)
	{
		params_.push_back(expr.get());
		expr.release();
	}



	Function::Function(std::unique_ptr<Signature> signature,
						std::unique_ptr<Block> body,
						Location start,
						Location finish) noexcept
		: LocatedInFile(start, finish)
		, signature_(signature.release())
		, body_(body.release())
	{
		assert(signature_);
		assert(body_);
	}

	Function::~Function()
	{
		delete signature_;
		delete body();
	}

	std::string const & Function::name() const noexcept
	{ return signature_->name(); }

	Type Function::return_type() const noexcept
	{ return signature_->return_type(); }

	Type Function::type_at(std::size_t index) const noexcept
	{ return signature_->at(index).first; }

	std::string const & Function::name_at(std::size_t index) const noexcept
	{ return signature_->at(index).second; }

	Block * Function::body() noexcept
	{ return body_; }

	Block const * Function::body() const noexcept
	{ return body_; }



	Variable::Variable(Type type,
						std::string name,
						Location start,
						Location finish)
		: LocatedInFile(std::move(start), std::move(finish))
		, type_(type)
		, name_(name)
		, owner_(nullptr)
	{ }

	std::string const & Variable::name() const noexcept
	{ return name_; }

	Type Variable::type() const noexcept
	{ return type_; }

	Scope * Variable::owner() noexcept
	{ return owner_; }

	Scope const * Variable::owner() const noexcept
	{ return owner_; }

	void Variable::set_owner(Scope * scope) noexcept
	{ owner_ = scope; }


}
