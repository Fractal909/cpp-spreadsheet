#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


TextImpl::TextImpl(std::string str)
    :text_(str)
{
}

FormulaImpl::FormulaImpl(std::string str)
    :formula_(ParseFormula(str))
{
}

std::string TextImpl::GetText() {
    return text_;
}

FormulaInterface* FormulaImpl::GetFormulaPtr() {
    return formula_.get();
}


// Реализуйте следующие методы

Cell::Cell(SheetInterface& sheet)
    :sheet_(sheet)
{

}

Cell::~Cell() {

}

void Cell::Set(std::string text) {

    if (text.length() > 1 && text.at(0) == '=') {
        impl_ = std::make_unique<FormulaImpl>(text.substr(1));
    }
    else {
        impl_ = std::make_unique<TextImpl>(text);
    }
}

void Cell::Clear() {

}

Cell::Value Cell::GetValue() const {

    if (cache_valid_) {
        return cache_;
    }

    if (auto ptr = dynamic_cast<FormulaImpl*>(impl_.get())) {

        auto result = ptr->GetFormulaPtr()->Evaluate(sheet_);
        if (auto double_ptr = std::get_if<double>(&result)) {

            cache_ = *double_ptr;
            cache_valid_ = true;
            return *double_ptr;
        }
        else if (auto formula_error_ptr = std::get_if<FormulaError>(&result)) {

            cache_ = *formula_error_ptr;
            cache_valid_ = true;
            return *formula_error_ptr;
        }
        //
        return -1.0;
    }
    if (auto ptr = dynamic_cast<TextImpl*>(impl_.get())) {
        std::string str = ptr->GetText();
        if (!str.empty() && str.at(0) == '\'') {
            return str.substr(1);
        }

        cache_ = ptr->GetText();
        cache_valid_ = true;
        return ptr->GetText();
    }
    //
    return -1.0;
}
std::string Cell::GetText() const {
    if (auto ptr = dynamic_cast<FormulaImpl*>(impl_.get())) {
        return '=' + ptr->GetFormulaPtr()->GetExpression();
    }
    if (auto ptr = dynamic_cast<TextImpl*>(impl_.get())) {
        return ptr->GetText();
    }
    return "ERROR";
}


std::vector<Position> Cell::GetReferencedCells() const {
    if (auto ptr = dynamic_cast<FormulaImpl*>(impl_.get())) {
        return ptr->GetFormulaPtr()->GetReferencedCells();
    }
    else {
        return {};
    }
}

std::vector<Position> Cell::GetDependentCells() const {
    std::vector<Position> result;
    for (auto& cell : dependent_cells_) {
        result.push_back(cell);
    }
    return result;
}

void Cell::AddDependentCell(Position cell) const {
    dependent_cells_.insert(cell);
}
void Cell::RemoveDependentCell(Position cell) const {
    dependent_cells_.erase(cell);
}
void Cell::InvalidateCache() const {
    cache_valid_ = false;
}