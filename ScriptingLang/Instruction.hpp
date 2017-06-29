#pragma once

#include "pch.hpp"
#include "Location.hpp"

namespace Plang
{
    //necessary?
    template <typename T>
    class NonOwningRef
    {
    public:
        NonOwningRef() : ptr(nullptr) { }
        NonOwningRef(T& value) : ptr(&value) { }
        NonOwningRef(T* value) : ptr(value) { }
        ~NonOwningRef() { }

        inline T* operator ->() { return ptr; }
        inline T& operator *() { return *ptr; }
        inline T& operator *() const { return *ptr; }

        inline bool operator ==(std::nullptr_t null) const
        {
            return ptr == null;
        }
        inline bool operator ==(const T& other) const //reference compare
        {
            return ptr == &other;
        }
        inline bool operator ==(T* other) const //reference compare
        {
            return ptr == other;
        }
        inline bool operator !=(std::nullptr_t null) const
        {
            return ptr != null;
        }
        inline bool operator !=(const T& other) const //reference compare
        {
            return ptr != &other;
        }
        inline bool operator !=(T* other) const //reference compare
        {
            return ptr != other;
        }

    protected:
        T* ptr;
    };

    enum class InstructionType
    {
        Unknown,
        Program,
        Statement, //internal only

        Int,
        Float,
        String,

        Identifier,
        Accessor,

        List,
        Block,
        Tuple,

        Call,
        Expression
    };

    class Instruction;
    class InstructionFacade;

    using TInt = int64_t;
    using TFloat = double;
    using TList = std::vector<Instruction>;

    //instructions created for storage in the AST.
    //These are used by the runtime, but should be static after parsing
    class Instruction
    {
    public:

        using ValueType = std::variant<std::nullptr_t, TInt, TFloat, std::string, TList>;

        Instruction() = default;
        Instruction(InstructionType type) : type(type) { }
        Instruction(InstructionType type, const ValueType& value) : type(type), value(value) { }

        virtual ~Instruction() = default;

        Location location;
        NonOwningRef<Instruction> parent; //nullptr if root instruction
        ValueType value;
        InstructionType type;

        std::string TypeName() const;

        template <class TFacade>
        std::enable_if_t<std::is_convertible_v<TFacade, InstructionFacade>, TFacade>& As()
        {
            return static_cast<TFacade&>(*this);
        }

        operator std::string() const;
	};

    class InstructionFacade : public Instruction
    {
    protected:
        InstructionFacade(InstructionType type, const ValueType& value)
            : Instruction(type, value) { }
    };

    namespace Instructions
    {
        class List : public InstructionFacade
        {
        public:
            inline Instruction& operator [](size_t index)
            {
                auto& items = std::get<TList>(value);
                return items.at(index);
            }
            inline Instruction operator [](size_t index) const
            {
                return operator[](index);
            }

            inline Instruction& Add(const Instruction& instruction)
            {
                auto& items = std::get<TList>(value);
                items.push_back(instruction);
                return items.back();
            }
            inline Instruction& Insert(const Instruction& instruction, size_t index = 0)
            {
                auto& items = std::get<TList>(value);
                return *items.insert(items.begin() + index, instruction);
            }
            inline bool Remove(const Instruction& instruction)
            {
                auto& items = std::get<TList>(value);
                for (auto i = items.begin(); i != items.end(); ++i)
                {
                    if (&*i == &instruction)
                    {
                        items.erase(i);
                        return true;
                    }
                }
                return false;
            }

            inline size_t Count() const
            {
                auto& items = std::get<TList>(value);
                return items.size();
            }

            inline Instruction& First()
            {
                auto& items = std::get<TList>(value);
                return items.front();
            }

            inline Instruction& Last()
            {
                auto& items = std::get<TList>(value);
                return items.back();
            }
        };

        class Call : public InstructionFacade
        {
        public:
            Call(const Instruction& callee, std::initializer_list<Instruction> arguments)
                : InstructionFacade(InstructionType::Call, TList{ callee, Instruction(InstructionType::Tuple, TList(arguments)) }) { }
            Call(const Instruction& callee, const Instruction& arguments)
                : InstructionFacade(InstructionType::Call, TList{ callee, arguments }) { }

            Instruction& Callee()
            {
                auto& def = std::get<TList>(value);
                return def.at(0);
            }

            Instruction& Arguments()
            {
                auto& def = std::get<TList>(value);
                return def.at(1);
            }
        };

        class Expression : public InstructionFacade
        {
        public:
            Expression(const Instruction& arguments, const Instruction& body)
                : InstructionFacade(InstructionType::Expression, TList{ arguments, body }) { }

            Instruction& Arguments()
            {
                auto& def = std::get<TList>(value);
                return def.at(0);
            }

            Instruction& Body()
            {
                auto& def = std::get<TList>(value);
                return def.at(1);
            }
        };
    }

    std::ostream& operator << (std::ostream& stream, const Plang::Instruction& instruction);
};

//todo: proper string deconstruction (possibly store all strings in a set)
