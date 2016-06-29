#pragma once
#ifndef _TASK_HPP_
#define _TASK_HPP_
class Task
{
public:
	virtual void Run() = 0;
	virtual ~Task(){};
};
#endif

