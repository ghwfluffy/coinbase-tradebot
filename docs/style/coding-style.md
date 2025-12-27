# Coding Style

Snapshot of the conventions used in this codebase (indentation, braces, naming, ownership, comments, namespaces, and the "Rule of 5").

## Layout & Whitespace
- Indentation: 4 spaces; no tabs.
- Braces: Allman style (opening brace on its own line for functions, classes, switches, conditionals).
- Parameters: wrap long parameter lists one-per-line aligned on the indent.
- Control blocks: always brace multi-line bodies; single-line bodies are also typically braced for clarity.
- Spacing: prefer blank lines between logical blocks; avoid trailing whitespace. See `docs/style/whitespace.md`.

## Naming
- Types and classes: PascalCase (`TradeBot`, `DataController`, `OrderPairStateMachine`).
- Functions/methods: lowerCamelCase (`initMock`, `addProcessor`, `getCtx`); avoid snake_case for new functions.
- Variables: lowerCamelCase for locals/parameters (`mock`, `bot`, `version`, `ctx`); uppercase snake for constants/macros (`WEBSOCKETPP_INCLUDE`).
- Enums/states: PascalCase values (`Pending`, `BuyActive`, `Holding`).

## Namespacing
- Headers live under the `gtb` namespace; translation units often `using namespace gtb;` locally, but headers should avoid `using namespace`.
- Prefer fully-qualified names instead of 'using' for anything outside of the gtb namespace.

## Comments
- Brief, intent-focused comments sparingly; code is expected to be self-explanatory.
- Block comments before non-obvious sections; avoid inline noise comments.
- Keep ASCII-only unless there is a specific reason otherwise.

## Scoping & Structure
- Limit variable scope to the smallest needed block; initialize near first use.
- Prefer `const` where possible and pass by reference for non-owning inputs.
- Organize functions with helper statics in anonymous namespaces for TU-local utilities (`initMock`, `initProd` pattern).

## Dependency Injection
- Lifetime rule: anything that outlives the class instance should be injected through the constructor as a reference (e.g., `BotContext &ctx` held by traders/processors).
- Configuration: parameters that tune behavior can be passed in the constructor or set via explicit setters; prefer whatever results in the cleanest instantiation of the object.
- Constructor style: definitions in `.cpp` files follow the standard indent/brace pattern and member initialization layout:

```cpp
SpreadTrader::SpreadTrader(
    BotContext &ctx,
    Config confIn)
        : OrderPairTrader(ctx, confIn)
        , conf(std::move(confIn))
{
}
```

## Memory Management
- Default to RAII and standard smart pointers:
  - `std::unique_ptr` for ownership transfer (`TradeBot::addSource`, `addProcessor`).
  - References or raw pointers only for non-owning access (`BotContext &ctx`).
- Avoid manual `new`/`delete`; rely on constructors/destructors and standard containers.
- Prefer move semantics to avoid copies when transferring ownership.

## The Rule of 5
- Always declare the 5 in the same order as all the other classes in the code base.
- For types that manage resources or should not be copied, explicitly declare or delete the special members (ctor/dtor/copy/move/assign). Examples: `TradeBot` deletes copy/move where ownership should not be duplicated.
- If a type is trivially movable/copyable, rely on defaults; otherwise spell out the intended ownership semantics.

## Error Handling & Logging
- Use lightweight logging helpers (`log::info`, `log::error`) for runtime diagnostics.

## Integer & Units Discipline
- Use the strong-typed integer wrappers (`usd_t`, `btc_t`, `pp_t`, `utime_t`) and helpers from `IntegerUtils` instead of raw integers for domain values.
- Conversions to/from external formats (DB, strings) should go through the typed helpers to avoid unit bugs.

## Formatting Examples
- Function definition:
  ```cpp
  bool AlgorithmFactory::provision(
      gtb::TradeBot &bot,
      unsigned int version,
      bool mock)
  {
      switch (version)
      {
          ...
      }
  }
  ```
- Control flow:
  ```cpp
  if (mock)
      initMock(bot);
  else
      initProd(bot);
  ```
