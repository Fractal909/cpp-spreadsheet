#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {

    if (!CheckPosition(pos)) {
        throw InvalidPositionException("Invalid_Pos");
    }

    if (GetCell(pos)) {
        cells_.at(pos)->Set(text, pos);
    }
    else {
        auto new_cell = cells_.emplace(pos, std::make_unique<Cell>(*this)).first->second.get();
        new_cell->Set(text, pos);
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (CheckPosition(pos)) {
        auto res = cells_.find(pos);
        if (res != cells_.end()) {
            return res->second.get();
        }
        else {
            return nullptr;
        }
    }
    else {
        throw InvalidPositionException("Invalid_Pos");
    }
}

CellInterface* Sheet::GetCell(Position pos) {
    if (CheckPosition(pos)) {
        auto res = cells_.find(pos);
        if (res != cells_.end()) {
            return res->second.get();
        }
        else {
            return nullptr;
        }
    }
    else {
        throw InvalidPositionException("Invalid_Pos");
    }
}

void Sheet::ClearCell(Position pos) {
    if (CheckPosition(pos)) {
        cells_.erase(pos);
    }
    else {
        throw InvalidPositionException("Invalid_Pos");
    }
}

Size Sheet::GetPrintableSize() const {
    Size result;

    for (const auto& cell : cells_) {
        if (cell.first.col + 1 > result.cols) {
            result.cols = cell.first.col + 1;
        }
        if (cell.first.row + 1 > result.rows) {
            result.rows = cell.first.row + 1;
        }
    }
    return result;
}

void Sheet::PrintValues(std::ostream& output) const {
    Size size = GetPrintableSize();

    for (int row = 0; row < size.rows; ++row) {

        for (int col = 0; col < size.cols; ++col) {

            if (auto cell_ptr = GetCell({ row, col })) {
                auto val = cell_ptr->GetValue();
                if (double* double_ptr = std::get_if<double>(&val)) {
                    output << *double_ptr;
                }
                else if (std::get_if<std::string>(&val)) {
                    output << std::get<std::string>(cell_ptr->GetValue());
                }
                else {
                    output << "#ARITHM!";
                }
            }
            if (col < size.cols - 1) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    Size size = GetPrintableSize();

    for (int row = 0; row < size.rows; ++row) {

        for (int col = 0; col < size.cols; ++col) {

            if (auto cell_ptr = GetCell({ row, col })) {
                output << cell_ptr->GetText();
            }
            if (col < size.cols - 1) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

bool Sheet::CheckPosition(const Position& pos) const {
    return pos.col >= 0 && pos.row >= 0 && pos.col < Position::MAX_COLS && pos.row < Position::MAX_ROWS;
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}