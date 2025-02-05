#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>

class Impl {
public:
    virtual ~Impl() = default;
private:
};

class EmptyImpl : public Impl {

};

class TextImpl : public Impl {
public:
    TextImpl(std::string str);
    std::string GetText();
private:
    std::string text_;
};

class FormulaImpl : public Impl {
public:
    FormulaImpl(std::string str);
    FormulaInterface* GetFormulaPtr();
private:
    std::unique_ptr<FormulaInterface> formula_;
};

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet);
    ~Cell();

    void Set(std::string text, Position pos);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

    std::vector<Position> GetDependentCells() const override;
    void AddDependentCell(Position pos) const override;
    void RemoveDependentCell(Position pos) const override;

private:
    bool CheckCyclicDependence(CellInterface* cell);
    void InvalidateCellsCache() const override;
    std::unique_ptr<Impl> impl_;
    SheetInterface& sheet_;

    mutable std::unordered_set<Position, PosHash> dependent_cells_;
    mutable Value cache_;
    mutable bool cache_valid_ = false;
};
