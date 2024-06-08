#include "formula.h"



#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression):ast_(ParseFormulaAST(expression)){}
    Value Evaluate(const SheetInterface& sheet) const override{
        Value value;
        try {
            value = ast_.Execute(sheet);
        } catch (const FormulaError::Category &err) {
            value = err;
        }
        return value;
    }
    std::string GetExpression() const override{
        std::ostringstream out;
        ast_.PrintFormula(out);
        return out.str();
    }

    std::vector<Position> GetReferencedCells() const override{
        std::vector<Position>referenced_cells;
        std::set<Position>sett;
        auto cells = ast_.GetCells();
        for(auto cell: cells){
            sett.insert(cell);
        }
        for(auto cell:sett){
            referenced_cells.push_back(cell);
        }
        
        return referenced_cells;
    }
private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try{
        return std::make_unique<Formula>(std::move(expression));
    }
    catch(...){
        throw FormulaException("wrong expression formula");
    }
    
}


FormulaError::FormulaError(Category category) : category_(category) {}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return ToString() == rhs.ToString();
}

std::string_view FormulaError::ToString() const {
    switch (category_) {
    case Category::Ref:
        return "#REF!"sv;
    case Category::Value:
        return "#VALUE!"sv;
    case Category::Arithmetic:
        return "#DIV/0!"sv;
    default:
        assert(false);
        return "";
    }
}
