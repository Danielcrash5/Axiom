# TODO

## Prefab-System

Die Prefab-Arbeiten aus den Asset-System-Anforderungen sind bewusst zurückgestellt:

- `SceneEntity::prefabInstance` auf `std::optional<PrefabInstanceData>` umstellen.
- `SceneEntity::ToJson` und `FromJson` an die optionale Prefab-Instanz anpassen.
- Einen AddressSanitizer-Test für wiederholte Kopien und Reallokationen von `std::vector<SceneEntity>` ergänzen.
- `SceneManager::InstantiatePrefab` mit `entt::registry&` implementieren.
- `SceneManager::ReloadPrefab` implementieren: Prefab-Instanzen finden, Daten neu laden und nur nicht überschriebenen Komponenten aktualisieren.
- Einen Test für zwei Prefab-Instanzen mit Override und Reload ergänzen.
