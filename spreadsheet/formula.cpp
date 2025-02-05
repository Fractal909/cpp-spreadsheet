#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <vector>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}

namespace {
    class Formula : public FormulaInterface {
    public:
        // Реализуйте следующие методы:
        explicit Formula(std::string expression) try
            : ast_(ParseFormulaAST(expression)) {
        }
        catch (const std::exception& exc) {
            throw FormulaException("BAD_EXPR");
        }

        Value Evaluate(SheetInterface& sheet) const override {
            try {
                return ast_.Execute(sheet);
            }
            catch (const FormulaError& exc) {
                return FormulaError(exc);
            }
        }

        std::string GetExpression() const override {
            std::stringstream out;
            ast_.PrintFormula(out);
            return out.str();
        }

        std::vector<Position> GetReferencedCells() const override {

            std::vector<Position> result;
            std::set<Position> tmp;
            for (auto& cell : ast_.GetCells()) {
                tmp.insert(cell);
            }
            for (auto& cell : tmp) {
                result.push_back(cell);
            }
            return result;
        }

    private:
        FormulaAST ast_;
    };
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}