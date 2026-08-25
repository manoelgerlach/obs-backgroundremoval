// SPDX-FileCopyrightText: 2026 obs-backgroundremoval contributors
// SPDX-FileCopyrightText: 2026 Manoel Gerlach <mail@manoel.us>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#if __has_include(<onnxruntime/onnxruntime_cxx_api.h>)
#include <onnxruntime/onnxruntime_cxx_api.h>
#elif __has_include(<onnxruntime_cxx_api.h>)
#include <onnxruntime_cxx_api.h>
#else
#error "onnxruntime_cxx_api.h was not found"
#endif

#if __has_include(<onnxruntime/dml_provider_factory.h>)
#include <onnxruntime/dml_provider_factory.h>
#elif __has_include(<dml_provider_factory.h>)
#include <dml_provider_factory.h>
#elif __has_include(<onnxruntime/core/providers/dml/dml_provider_factory.h>)
#include <onnxruntime/core/providers/dml/dml_provider_factory.h>
#else
#error "dml_provider_factory.h was not found"
#endif

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {

std::vector<int64_t> getFixedShape(const Ort::Session &session, size_t index, bool input)
{
	Ort::TypeInfo typeInfo = input ? session.GetInputTypeInfo(index) : session.GetOutputTypeInfo(index);
	auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
	if (tensorInfo.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
		throw std::runtime_error("MediaPipe smoke test requires float tensors");
	}

	std::vector<int64_t> shape = tensorInfo.GetShape();
	for (int64_t &dimension : shape) {
		if (dimension <= 0) {
			dimension = 1;
		}
	}
	return shape;
}

size_t getElementCount(const std::vector<int64_t> &shape)
{
	return std::accumulate(shape.begin(), shape.end(), size_t{1},
			       [](size_t count, int64_t dimension) { return count * static_cast<size_t>(dimension); });
}

} // namespace

int main(int argc, char **argv)
{
	if (argc != 2) {
		std::cerr << "Usage: DirectMLMediaPipeSmokeTest <mediapipe.onnx>\n";
		return 2;
	}

	try {
		Ort::Env environment(ORT_LOGGING_LEVEL_WARNING, "directml-mediapipe-smoke-test");
		Ort::SessionOptions sessionOptions;
		sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
		sessionOptions.DisableMemPattern();
		sessionOptions.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
		sessionOptions.AddConfigEntry("session.disable_cpu_ep_fallback", "1");

		const void *providerApi = nullptr;
		Ort::ThrowOnError(Ort::GetApi().GetExecutionProviderApi("DML", ORT_API_VERSION, &providerApi));
		const auto *dmlApi = static_cast<const OrtDmlApi *>(providerApi);

		OrtDmlDeviceOptions deviceOptions{};
		deviceOptions.Preference = OrtDmlPerformancePreference::HighPerformance;
		deviceOptions.Filter = OrtDmlDeviceFilter::Gpu;
		Ort::ThrowOnError(dmlApi->SessionOptionsAppendExecutionProvider_DML2(sessionOptions, &deviceOptions));

		const std::filesystem::path modelPath(argv[1]);
		Ort::Session session(environment, modelPath.c_str(), sessionOptions);
		Ort::AllocatorWithDefaultOptions allocator;
		Ort::AllocatedStringPtr inputName = session.GetInputNameAllocated(0, allocator);
		Ort::AllocatedStringPtr outputName = session.GetOutputNameAllocated(0, allocator);

		const std::vector<int64_t> inputShape = getFixedShape(session, 0, true);
		const std::vector<int64_t> outputShape = getFixedShape(session, 0, false);
		std::vector<float> inputValues(getElementCount(inputShape), 0.0f);
		std::vector<float> outputValues(getElementCount(outputShape), 0.0f);
		Ort::MemoryInfo memoryInfo =
			Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtDeviceAllocator, OrtMemType::OrtMemTypeDefault);

		std::array<Ort::Value, 1> inputTensors{Ort::Value::CreateTensor<float>(
			memoryInfo, inputValues.data(), inputValues.size(), inputShape.data(), inputShape.size())};
		std::array<Ort::Value, 1> outputTensors{Ort::Value::CreateTensor<float>(
			memoryInfo, outputValues.data(), outputValues.size(), outputShape.data(), outputShape.size())};
		const std::array<const char *, 1> inputNames{inputName.get()};
		const std::array<const char *, 1> outputNames{outputName.get()};

		for (int iteration = 0; iteration < 100; ++iteration) {
			session.Run(Ort::RunOptions{nullptr}, inputNames.data(), inputTensors.data(),
				    inputTensors.size(), outputNames.data(), outputTensors.data(),
				    outputTensors.size());

			const std::vector<int64_t> currentShape =
				outputTensors[0].GetTensorTypeAndShapeInfo().GetShape();
			if (currentShape != outputShape) {
				throw std::runtime_error("DirectML output shape changed between inferences");
			}
			if (!std::all_of(outputValues.begin(), outputValues.end(),
					 [](float value) { return std::isfinite(value); })) {
				throw std::runtime_error("DirectML produced a non-finite MediaPipe output");
			}
		}

		std::cout << "DirectML MediaPipe smoke test completed 100 consecutive inferences\n";
		return 0;
	} catch (const Ort::Exception &exception) {
		const std::string_view message(exception.what());
		if (message.find("No devices detected that match the filter criteria") != std::string_view::npos) {
			std::cerr << "DirectML MediaPipe smoke test skipped: no matching GPU device was detected\n";
			return 77;
		}

		std::cerr << "DirectML MediaPipe smoke test failed: " << exception.what() << '\n';
		return 1;
	} catch (const std::exception &exception) {
		std::cerr << "DirectML MediaPipe smoke test failed: " << exception.what() << '\n';
		return 1;
	}
}
