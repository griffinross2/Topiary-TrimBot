Import("env")

# Install packages
try:
    import cairo
except ImportError:
    env.Execute("$PYTHONEXE -m pip install pycairo")
    
try:
    import fontTools
except ImportError:
    env.Execute("$PYTHONEXE -m pip install fontTools")