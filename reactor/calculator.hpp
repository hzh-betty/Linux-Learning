#pragma once
#include "protocol.hpp"

enum
{
    DivZero = 1,
    ModZero,
    OtherOper
};
class Calculator
{
public:
    static Response calculator(const Request &req)
    {
        Response resp(0, 0);
        switch (req.op_)
        {
        case '+':
            resp.result_ = req.x_ + req.y_;
            break;
        case '-':
            resp.result_ = req.x_ - req.y_;
            break;
        case '*':
            resp.result_ = req.x_ * req.y_;
            break;
        case '/':
            if (req.y_ == 0)
            {
                resp.code_ = DivZero;
            }
            else
            {
                resp.result_ = req.x_ / req.y_;
            }
            break;
        case '%':
            if (req.y_ == 0)
            {
                resp.code_ = ModZero;
            }
            else
            {
                resp.result_ = req.x_ % req.y_;
            }
            break;
        default:
            resp.code_ = OtherOper;
            break;
        }
        return resp;
    }
};