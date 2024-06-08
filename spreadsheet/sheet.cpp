#include "sheet.h"
#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;


Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if(pos.row < 0 || pos.col<0 || pos.col >= Position::MAX_COLS || pos.row >= Position::MAX_ROWS)
    {
        throw InvalidPositionException("wrong pos");
    }
    
    if (pos.row>= size_.rows) {
        size_.rows = pos.row+1;
        sheet_.resize(size_.rows);
    }
    if (pos.col >= size_.cols) {
        size_.cols = pos.col+1;
    }

    if (sheet_[pos.row].size() < static_cast<size_t>(size_.cols)) {
        sheet_[pos.row].resize(size_.cols);
    }

    CheckCyclicDependency(text, pos);
    
    Cell cell(*this);
    cell.Set(text);
    
    sheet_[pos.row][pos.col] = std::make_unique<Cell>(std::move(cell));
    Position pos_new;
    
    if(text[0] == '=')
    {
        text = text.substr(1, text.size()-1);
        Position tmp_pos = Position::FromString(text);
        if(tmp_pos.col != -1 && tmp_pos.row != -1)
        {
            if(GetCell(tmp_pos) == nullptr){
                pos_new = tmp_pos;
                SetCell(pos_new, "0");
            }
            else{
                pos_new = tmp_pos;
            }
            
            sheet_[pos.row][pos.col]->AddDepedencyFor(GetConcreteCell(pos_new));
            
            sheet_[pos_new.row][pos_new.col]->AddDepedencyFrom(GetConcreteCell(pos));
        }
    }
    
}


void Sheet::CheckCyclicDependency(std::string text, Position pos){
    try{
        if(!text.empty()){
            if(text[0] == '='){
                text = text.substr(1);
            }
            auto formula = ParseFormula(text);
            for(auto cell: formula->GetReferencedCells()){
                CellInterface* next_cell = GetCell(cell);
                if(pos == cell){
                    throw CircularDependencyException("Circular dependecy");
                }
                if(next_cell != nullptr){
                    CheckCyclicDependency(next_cell->GetText(), pos);
                }
            }
        }
    }
    catch(FormulaException&){
        
    }
}

const Cell* Sheet::GetConcreteCell(Position pos) const{
    if(pos.row < 0 || pos.col<0 || pos.col >= Position::MAX_COLS || pos.row >= Position::MAX_ROWS)
    {
        throw InvalidPositionException("wrong pos");
    }
    if (sheet_.size() > static_cast<size_t>(pos.row)) {
        if (sheet_[pos.row].size() <= static_cast<size_t>(pos.col)) {
            return nullptr;
        }
    }
    else {
        return nullptr;
    }
    if (sheet_[pos.row][pos.col]->GetText().empty()) {
        return nullptr;
    }
    return sheet_[pos.row][pos.col].get();
}
Cell* Sheet::GetConcreteCell(Position pos){
    if(pos.row < 0 || pos.col<0 || pos.col >= Position::MAX_COLS || pos.row >= Position::MAX_ROWS)
    {
        throw InvalidPositionException("wrong pos");
    }
    if (sheet_.size() > static_cast<size_t>(pos.row)) {
        if (sheet_[pos.row].size() <= static_cast<size_t>(pos.col)) {
            return nullptr;
        }
    }
    else {
        return nullptr;
    }
    if (sheet_[pos.row][pos.col]->GetText().empty()) {
        //return nullptr;
        return nullptr;
    }
    return sheet_[pos.row][pos.col].get();
}


const CellInterface* Sheet::GetCell(Position pos) const {
    if(pos.row < 0 || pos.col<0 || pos.col >= Position::MAX_COLS || pos.row >= Position::MAX_ROWS)
    {
        throw InvalidPositionException("wrong pos");
    }
    if (sheet_.size() > static_cast<size_t>(pos.row)) {
        if (sheet_[pos.row].size() <= static_cast<size_t>(pos.col)) {
            return nullptr;
        }
    }
    else {
        return nullptr;
    }
    if (sheet_[pos.row][pos.col]->GetText().empty()) {
        //return nullptr;
        return nullptr;
    }
    return sheet_[pos.row][pos.col].get();
}

CellInterface* Sheet::GetCell(Position pos) {
    if(pos.row < 0 || pos.col<0 || pos.col >= Position::MAX_COLS || pos.row >= Position::MAX_ROWS)
    {
        throw InvalidPositionException("wrong pos");
    }
    if (sheet_.size() > static_cast<size_t>(pos.row)) {
        if (sheet_[pos.row].size() <= static_cast<size_t>(pos.col)) {
            return nullptr;
        }
    }
    else {
        return nullptr;
    }
    if (sheet_[pos.row][pos.col] == nullptr) {
        return nullptr;
    }
    return sheet_[pos.row][pos.col].get();
}

void Sheet::UpdateSize(Position& pos){
    Size new_size = {0, 0};
    if(pos.col == 0 && pos.row == 0)
    {
        size_ = new_size;
        return;
    }
    for(int i =0; i <static_cast<int>(sheet_.size());i++)
    {
        for(int j =0; j < static_cast<int>(sheet_[i].size()); j++ ){
            if(sheet_[i][j] != nullptr){
                new_size.rows = i;
                new_size.cols = j;
            }
        }
    }
    new_size.rows++;
    new_size.cols++;
    size_ = new_size;
}

void Sheet::ClearCell(Position pos) {
    if(pos.row < 0 || pos.col<0 || pos.col >= Position::MAX_COLS || pos.row >= Position::MAX_ROWS)
    {
        throw InvalidPositionException("wrong pos");
    }

    if (sheet_.size() > static_cast<size_t>(pos.row) && (GetCell(pos)!= nullptr)) {
        if (sheet_[pos.row].size() > static_cast<size_t>(pos.col)) {
            sheet_[pos.row][pos.col]->Clear();
            sheet_[pos.row][pos.col].release();
            UpdateSize(pos);
        }
    }


}

Size Sheet::GetPrintableSize() const {
    return size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i < size_.rows; ++i) {
        for (int j = 0; j < size_.cols; ++j) {
            if (sheet_[i].size() > static_cast<size_t>(j)) {
                if (sheet_[i][j] != nullptr) {
                    const auto cell = sheet_[i][j]->GetValue();

                    if (std::holds_alternative<double>(cell)) {
                        output << std::get<double>(cell);
                    }
                    else if (std::holds_alternative<std::string>(cell)) {
                        output << std::get<std::string>(cell);
                    }
                    else if (std::holds_alternative<FormulaError>(cell)) {
                        output << "#ARITHM!";
                    }
                }
            }
            if(j != size_.cols -1){
                output << '\t';
            }
        }
        output << "\n";
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i < size_.rows;++i) {
        for(int j =0; j < size_.cols; j++)
        {
            if (sheet_[i].size() > static_cast<size_t>(j)) {
                if(sheet_[i][j] != nullptr){
                    output << sheet_[i][j]->GetText();
                }
            }
            if(j != size_.cols -1){
                output << '\t';
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
