#pragma once

#include "cell.h"
//#include "common.h"
#include <optional>
#include <functional>
#include <map>

class Sheet : public SheetInterface {
public:
    
    //Sheet(Sheet& sheet);
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;
    void UpdateSize(Position& pos);

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;
    const Cell* GetConcreteCell(Position pos) const;
    Cell* GetConcreteCell(Position pos);
    //void CheckCyclicDependency(std::string text);
    void CheckCyclicDependency(std::string text, Position pos);
    std::optional<std::pair<size_t, size_t>> FindElementPosition(int value);
    // Можете дополнить ваш класс нужными полями и методами

private:
    // Можете дополнить ваш класс нужными полями и методами
    Size size_;
//    std::vector<Size>old_sizes_;
//    std::unordered_map<Position, Size>map_old_sizes_;
    //std::map<Position, Size>_map_old_sizes_;
    std::vector<std::vector<std::unique_ptr<Cell>>> sheet_;
};
