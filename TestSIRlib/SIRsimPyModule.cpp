#include <Python.h>
#include <string>

#include "SIRSimRunner.h"

SIRSimRunner *sim;
static PyObject *
Configure(PyObject *self, PyObject *args)
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

    if (!PyArg_ParseTuple(args,
                          "siddlIIIIII",
                          &fileName,
                          &nTrajectories,
                          &λ,
                          &Ɣ,
                          &nPeople,
                          &ageMin,
                          &ageMax,
                          &ageBreak,
                          &tMax,
                          &Δt,
                          &pLength))
        return Py_BuildValue("i", false);

    sim = new SIRSimRunner::SIRSimRunner(string(fileName),
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
Run(PyObject *self, PyObject *args) {
    bool succ = true;
    int runType;

    if (!PyArg_ParseTuple(args, "i", &runType))
        return Py_BuildValue("i", false);

    if (runType == 0)
        succ = sim->SIRSimRunner::Run<RunType::Serial>();
    else if (runType == 1)
        succ = sim->SIRSimRunner::Run<RunType::Parallel>();
    else
        return Py_BuildValue("i", false);

    return Py_BuildValue("i", succ);
}

static PyObject *
Write(PyObject *self, PyObject *args) {
    bool succ = true;

    vector<string> filesWritten = sim->Write();

    if (!filesWritten.empty())
        return Py_BuildValue("i", true);
    else
        return Py_BuildValue("i", false);
}

static PyMethodDef SIRsimPyModuleMethods[] = {
    {"configure",  Configure, METH_VARARGS,
     "Configure the runner."},
    {"run",  Run, METH_VARARGS,
     "Run the runner."},
    {"write",  Configure, METH_VARARGS,
     "Write the file."},
    {NULL, NULL, 0, NULL}    /* sentinel */
};

PyMODINIT_FUNC
initSIRsimPyModule(void)
{
    (void) Py_InitModule("SIRsimPyModule", SIRsimPyModuleMethods);
}
