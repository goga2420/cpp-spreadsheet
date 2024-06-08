#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


Cell::Cell(SheetInterface& sheet) : sheet_(sheet),impl_(std::make_unique<FormulaImpl>(FormulaImpl(sheet))){}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
    } else if (text[0] == '=' && text.size() > 1) {
        try{
            ClearCache();
            impl_ = std::make_unique<FormulaImpl>(sheet_);
            impl_->Set(text.substr(1));
        }
        catch(...){
            throw FormulaException("wrong formula");
        }
        
    }
    else if (text[0] == '=' && text.size() == 1){
        impl_ = std::make_unique<TextImpl>();
        impl_->Set(std::move(text));
    }
    else {
        impl_ = std::make_unique<TextImpl>();
        impl_->Set(std::move(text));
    }
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetRefferencedCells();
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

void Cell::ClearCache(){
    cached_value_.reset();
    for(Cell* cell: refference_from){
        cell->ClearCache();
    }
}

Cell::Value Cell::GetValue() const {
    if(cached_value_.has_value()){
        return std::get<double>(cached_value_.value());
    }
    try{
        cached_value_ = std::get<double>(impl_->GetValue());
    }
    catch(...){
        
    }
    return impl_->GetValue();
}
std::string Cell::GetText() const {
    return impl_->GetText();
}
