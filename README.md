# Nova Chatbot - Project Overview and Architecture

Nova is a modular C++ chatbot system with SQLite-backed memory and a basic neural network-style response engine. It is designed to learn from users through a teaching mode, reinforce responses through feedback, and select the most appropriate reply using a combination of pattern matching, vector similarity, and fallback generation.

---

## File Structure (Overview)

```
src/
├── Core/
│   ├── main.cpp               # Entry point
│   ├── NeuralNet.cpp          # Vectorization, NN logic, cosine similarity
├── Humanizer/
│   ├── ContextTracker.cpp     # Handles context (TBD)
│   ├── ResponseSelector.cpp   # (Optional override for response logic)
│   ├── ResponseVariator.cpp   # Learns, selects, generates responses
│   ├── TopicExtractor.cpp     # Token extraction for topic matching
│   ├── WordVectorHelper.cpp   # Tokenization & word-level tools
├── Controller.cpp             # Handles frontend/backend interaction
├── utils.cpp                  # Utilities (e.g. string cleanup)
```

---

## Response Logic (ResponseVariator)

### `getResponse()` Decision Chain:
1. **Exact Match**: Look for known input in the `responses` table.
2. **Fuzzy Match**: Use Levenshtein distance to find a close match.
3. **Neural Network Generator**: Use vector similarity to guess a fitting response.
4. **Fallback**: Return default message if all fail.

### Feedback
- 👍 / 👎 buttons in GUI modify confidence in `responses.confidence`.
- Feedback updates are stored instantly in the SQLite DB.

### Teaching Mode
- User provides (topic, response) pair.
- Data is saved and passed to `NeuralNet::train()` for vector updates.

---

## Neural Network Design (NeuralNet.cpp)

This isn't a deep neural net; it's a lightweight, explainable model that treats text as word vectors and learns via cosine similarity and dot-product updates.

### Vectorization
- Each word is turned into a 3D or 100D vector (via model file or default).
- An input string is tokenized, and its word vectors are averaged and normalized.

### `vectorize()` Logic
1. Tokenize input.
2. Sum word vectors.
3. Divide by word count.
4. Normalize vector length (unit vector).

### Cosine Similarity
Used to compare how similar two vectors are.
```cpp
cos(A, B) = dot(A, B) / (||A|| * ||B||)
```
- Value between -1 and 1.
- 1 = identical direction (similar meaning), 0 = unrelated.

### Forward Pass (Predict)
```cpp
output[i] = input[i] * weight[i]  // Element-wise dot product
```
No layers, no activation functions. Just a linear transform.

### Loss Function (MSE)
```cpp
loss = mean((predicted - target)^2)
```
Used to see how far off the predicted vector is from the true one.

### Backpropagation
```cpp
gradient = (weight[i] - target[i]) * weight[i];
weight[i] -= learningRate * gradient;
```
- Applies a simple gradient descent step to shift embeddings closer.
- Very lightweight, fast training.

---

## Database
- `responses(topic, response, confidence)` - main learned data.
- `word_vectors(word, vector)` - stores embeddings for each word.
- Used for both learning and inference.

---

## Additional Features
- **`trainFromDatabaseForDev()`**: Bulk retrains NN from stored data.
- **`bulkTeachFromCSV()`**: Load CSV of responses and confidence scores.
- **Teaching suggestions** (future): `getFollowupSuggestion()` placeholder.

---

## Summary
Nova uses a hybrid approach:
- Hardcoded fallback.
- Symbolic matching.
- Fuzzy distance checks.
- Lightweight vector-based neural net for inference.

It blends learnability with efficiency and explainability, great for desktop use with SQLite.

---

## ✨ Made with ❤️ by [Dokeabj + My Beloved Ai]

