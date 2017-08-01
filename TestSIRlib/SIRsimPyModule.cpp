#include <Python.h>
#include <string>

#include "SIRSimRunner.h"

SIRSimRunner *sim;

static PyObject *
Configure(PyObject *_fileName,
          PyObject *_nTrajectories,
          PyObject *_λ,
          PyObject *_Ɣ,
          PyObject *_nPeople,
          PyObject *_ageMin,
          PyObject *_ageMax,
          PyObject *_ageBreak,
          PyObject *_tMax,
          PyObject *_Δt,
          PyObject *_pLength)
{
    char *       fileName;
    int          nTrajectories;
    double       λ;
    double       Ɣ;
    long         nPeople;
    unsigned int ageMin;
    unsigned int ageMax;
    unsigned int ageBreak;
    unsigned int tMax;
    unsigned int Δt;
    unsigned int pLength;

    if (!PyArg_ParseTuple(_fileName, "s", &fileName)           ||
        !PyArg_ParseTuple(_nTrajectories, "i", &nTrajectories) ||
        !PyArg_ParseTuple(_λ, "d", &λ)                         ||
        !PyArg_ParseTuple(_Ɣ, "d", &Ɣ)                         ||
        !PyArg_ParseTuple(_nPeople, "l", &nPeople)             ||
        !PyArg_ParseTuple(_ageMin, "I", &ageMin)               ||
        !PyArg_ParseTuple(_ageMax, "I", &ageMax)               ||
        !PyArg_ParseTuple(_ageBreak, "I", &ageBreak)           ||
        !PyArg_ParseTuple(_tMax, "I", &tMax)                   ||
        !PyArg_ParseTuple(_Δt, "I", &Δt)                       ||
        !PyArg_ParseTuple(_pLength, "I", &pLength))
        return Py_BuildValue("i", false);

    sim = new SIRSimRunner(string(fileName),
                           nTrajectories,
                           λ,
                           Ɣ,
                           nPeople,
                           ageMin,
                           ageMax,
                           ageBreak,
                           tMax,
                           Δt,
                           pLength);

    return Py_BuildValue("i", true);
}

using RunType = SIRSimRunner::RunType;

static PyObject *
Run(PyObject *_runType) {
    bool succ = true;
    int runType;

    if !(PyArg_ParseTuple(_runType, "i", &runType))
        return Py_BuildValue("i", false);

    if (runType == 0)
        succ = sim->Run<RunType::Serial>();
    else if (runType == 1)
        succ = sim->Run<RunType::Parallel>();
    else
        return Py_BuildValue("i", false);

    return Py_BuildValue("i", succ);
}

static PyObject *
Write(void) {
    bool succ = true;

    vector<string> filesWritten = sim->Write();

    if (!filesWritten.empty())
        return Py_BuildValue("i", true);
    else
        return Py_BuildValue("i", false);
}
