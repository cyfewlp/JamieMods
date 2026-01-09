# Documentation & Tagging Convention

This guide outlines the tagging hierarchy used in this vault. Following these rules allows us to generate automated dashboards and prevents information silos.

## 1. Tag Hierarchy

We use **Nested Tags** (e.g., `#parent/child`). This keeps the Tag Pane organized and allows for broad or specific filtering.

### A. Status Tags (Workflow)
Track the lifecycle of a task or bug.

| Tag | Description |
| :--- | :--- |
| `#status/todo` | New item, waiting to be started. |
| `#status/investigating` | Researching the root cause or feasibility. |
| `#status/doing` | Actively being worked on. |
| `#status/testing` | Fix/Feature complete, awaiting verification. |
| `#status/done` | Completed and closed. |
| `#status/paused` | Work stopped temporarily. |
| `#status/wontfix` | Cannot be implemented or not a bug. |

### B. Type Tags (Categorization)
Define what the note is about.

| Tag | Description |
| :--- | :--- |
| `#type/bug` | A defect in existing functionality. |
| `#type/feature` | A request for new functionality. |
| `#type/refactor` | Code improvement without changing behavior. |
| `#type/research` | Technical spikes or documentation research. |

### C. Priority Tags
| Tag | Description |
| :--- | :--- |
| `#p/high` | Critical / Blocker. |
| `#p/mid` | Standard priority. |
| `#p/low` | Nice to have / Minor UI tweak. |

---

## 2. Usage Examples

### Reporting a Bug
When you find a bug, create a new note (or a section) and add the tags at the top:
`#type/bug` `#status/todo` `#p/high`

### Linking Notes
Instead of just writing text, use **Backlinks** to connect modules:
* "Issue found in [[Auth-Module]] regarding token expiration."

## 3. Best Practices
1. **Case Sensitivity:** Use lowercase for all tags (Obsidian tags are case-insensitive, but lowercase is standard).
2. **No Spaces:** Use hyphens if needed (e.g., `#status/in-progress`), but nested tags (e.g., `#status/doing`) are preferred.
3. **Placement:** Place tags at the very top of the file or under a `# Tags` header for visibility.