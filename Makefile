CC		= /home/gnardin/bin/mpich-3.1.4/bin/mpicxx
OMPI_CXXFLAGS	= -I/home/gnardin/bin/boost-1.58.0/include -I/home/gnardin/bin/repasthpc-2.1/include -I/home/gnardin/bin/netcdf-4.2.1.1/include -I/home/gnardin/bin/mpich-3.1.4/include
OMPI_LDFLAGS	= -L/home/gnardin/bin/boost-1.58.0/lib -L/home/gnardin/bin/repasthpc-2.1/lib -L/home/gnardin/bin/netcdf-4.2.1.1/lib -L/home/gnardin/bin/mpich-3.1.4/lib
OMPI_LIBS	= -lboost_filesystem-mt -lboost_mpi-mt -lboost_serialization-mt -lboost_system-mt -lnetcdf -lnetcdf_c++ -lmpi -lrelogo-2.1 -lrepast_hpc-2.1 -ldl -lm

EXECF   = bin/trustCoalitionHPC
EXEC	= $(EXECF)

SRCDIR	= src
SRCS	= $(filter-out $(SRCDIR)/$(EXECF).cpp,$(wildcard $(SRCDIR)/*.cpp))
HEADERS	= $(wildcard $(SRCDIR)/*.h)

OBJDIR	= obj
OBJFS	= $(notdir $(SRCS:.cpp=.o))
OBJS	= $(addprefix $(OBJDIR)/,$(OBJFS))

Debug: $(EXEC)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) -std=c++11 $(OMPI_CXXFLAGS) -c $< -o $@

$(EXEC): $(OBJS) $(HEADERS)
	$(CC) -std=c++11 $(OMPI_CXXFLAGS) $(SRCDIR)/$(EXECF).cpp $(OMPI_LDFLAGS) $(OMPI_LIBS) $(OBJS) -o $(EXEC)

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	mkdir $(OBJDIR)
