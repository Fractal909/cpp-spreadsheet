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

void Cell::Set(std::string text, Position pos) {

    //Check repeat
    if (text == GetText()) {
        return;
    }

    //Update Dependent Cells
    for (const auto& cell : GetReferencedCells()) {
        sheet_.GetCell(cell)->RemoveDependentCell(pos);
    }

    auto backup = std::move(impl_);

    if (text.length() > 1 && text.at(0) == FORMULA_SIGN) {
        impl_ = std::make_unique<FormulaImpl>(text.substr(1));
    }
    else if (!text.empty()) {
        impl_ = std::make_unique<TextImpl>(text);
    }
    else {
        impl_ = std::make_unique<EmptyImpl>();
    }

    //Create missing cells
    for (const auto& cell : GetReferencedCells()) {
        if (!sheet_.GetCell(cell)) {
            sheet_.SetCell(cell, "");
        }
    }

    //Check Cyclic Dependence
    if (CheckCyclicDependence(this)) {
        impl_ = std::move(backup);
        throw CircularDependencyException("CircularDependency");
    }

    //Update Dependent Cells
    for (const auto& cell : GetReferencedCells()) {
        sheet_.GetCell(cell)->AddDependentCell(pos);
    }

    InvalidateCellsCache();
}

void Cell::Clear() {
    InvalidateCellsCache();
    impl_.reset();
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
        return 0.0;
    }
    if (auto ptr = dynamic_cast<TextImpl*>(impl_.get())) {
        std::string str = ptr->GetText();
        if (!str.empty() && str.at(0) == ESCAPE_SIGN) {
            return str.substr(1);
        }

        cache_ = ptr->GetText();
        cache_valid_ = true;
        return ptr->GetText();
    }
    //
    return 0.0;
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

void Cell::AddDependentCell(Position pos) const {
    dependent_cells_.insert(pos);
}
void Cell::RemoveDependentCell(Position pos) const {
    dependent_cells_.erase(pos);
}

bool Cell::CheckCyclicDependence(CellInterface* cell) {

    auto cells = cell->GetReferencedCells();

    for (const auto& cell : cells) {
        if (reinterpret_cast<CellInterface*>(this) == sheet_.GetCell(cell)) {
            return true;
        }
        else {
            return CheckCyclicDependence(sheet_.GetCell(cell));
        }
    }

    return false;
}

void Cell::InvalidateCellsCache() const {
    for (const auto& cell : GetDependentCells()) {
        sheet_.GetCell(cell)->InvalidateCellsCache();
    }
    cache_valid_ = false;
}