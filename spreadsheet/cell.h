#pragma once

#include "formula.h"
#include <sstream>
#include <optional>
#include <unordered_set>

class Cell : public CellInterface {
public:
    explicit Cell(SheetInterface& sheet);
     Cell(const Cell& ex):sheet_(ex.sheet_){
        Set(ex.GetText());
    }
    ~Cell();

    void Set(std::string text);
    void Clear();
    void ClearCache();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    bool IsReferenced() const;
    void AddDepedencyFrom(Cell* cell){
        refference_from.insert(cell);
    }
    void AddDepedencyFor(Cell* cell){
        refference_for.insert(cell);
    }
    

private:
    SheetInterface& sheet_;
    std::unordered_set<Cell*> refference_for;
    std::unordered_set<Cell*> refference_from;
    
    class Impl{
    public:
        virtual void Set(std::string text) =0;
        virtual void Clear() = 0;
        
        virtual std::vector<Position> GetRefferencedCells() = 0;

        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
    };
    
    class TextImpl: public Impl{
    public:
        void Set(std::string text) override{
            text_ = text;
        }
        void Clear() override {
            text_.clear();
        }
        
        std::vector<Position> GetRefferencedCells() override{
            return {};
        }
        
        Value GetValue() const override{
            if(text_[0] == '\'')
               return text_.substr(1);
            return text_;
        }
        
        std::string GetText() const override{
            return text_;
        }
    private:
        std::string text_;
    };
    
    class FormulaImpl: public Impl{
    public:

        FormulaImpl(SheetInterface& sheet):sheet_(sheet) {

        }
        void Set(std::string text) override {
            formula_ = ParseFormula(text);
        }

        void Set(std::string text,SheetInterface& sheet) {
            formula_ = ParseFormula(text);
        }
        void Clear() override {
            formula_.reset();
        }
        std::string GetText() const override {
            return "=" + formula_->GetExpression();
        }
        
        std::vector<Position> GetRefferencedCells() override{
            return formula_->GetReferencedCells();
        }

        Value GetValue() const override {
            FormulaInterface::Value value ;
            try{
                value = formula_->Evaluate(sheet_);
                return std::get<double>(value);
            }
            catch(...){
                return std::get<FormulaError>(value);
            }
        }
    private:
        std::unique_ptr<FormulaInterface> formula_;
        SheetInterface& sheet_;
        
    };
    class EmptyImpl: public Impl{
    public:
        std::string GetText() const override{
            return "";
        }
        
        void Clear() override {}
        void Set(std::string text) override {}
        
        std::vector<Position> GetRefferencedCells() override{
            return {};
        }
        
        Value GetValue() const override{
            return "";
        }
        
    };
    
    std::unique_ptr<Impl> impl_;
    mutable std::optional<CellInterface::Value> cached_value_;
};

