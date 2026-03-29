# Patch bgfx.cmake's bgfxToolUtils.cmake to fix broken $<IF> generator expression.
# Bug: uses "$<IF:$<CONFIG:Debug>:0,3>" (colon) instead of "$<IF:$<CONFIG:Debug>,0,3>" (comma)
file(READ cmake/bgfxToolUtils.cmake CONTENT)
string(REPLACE
    "$<IF:$<CONFIG:Debug>:0,3>"
    "$<IF:$<CONFIG:Debug>,0,3>"
    CONTENT "${CONTENT}"
)
file(WRITE cmake/bgfxToolUtils.cmake "${CONTENT}")
