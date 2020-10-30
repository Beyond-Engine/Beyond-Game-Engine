#ifndef BEYOND_GRAPHICS_D3D12_GPU_DEVICE_HPP
#define BEYOND_GRAPHICS_D3D12_GPU_DEVICE_HPP

#define GLM_FORCE_SSE42 1
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES 1
#define GLM_FORCE_LEFT_HANDED
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

#include <D3d12.h>
#include <direct.h>
#include <dxgi1_4.h>

#include <beyond/core/utils/panic.hpp>
#include <beyond/graphics/backend.hpp>
#include <beyond/platform/platform.hpp>

// Common Utils

inline std::vector<char> readFile(const std::string& filename)
{
  std::ifstream file(filename, std::ios::ate | std::ios::binary);
  bool exists = (bool)file;

  if (!exists || !file.is_open()) {
    throw std::runtime_error("failed to open file!");
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();

  return buffer;
};

// Renderer

namespace beyond::graphics::d3d12 {

class D3D12Device : public GPUDevice {
public:
  explicit D3D12Device(Window& window);
  ~D3D12Device() override;

  // Render onto the render target
  void render(GPUSwapchain swapchain) override;

  // Resize the window and internal data structures
  void resize(GPUSwapchain& swapchain, unsigned width,
              unsigned height) override;
  void initialize_resources(GPUSwapchain swapchain) override;
  void setup_commands() override;

  [[nodiscard]] auto create_swapchain(std::uint32_t width, std::uint32_t height)
      -> GPUSwapchain override;
  void destroy_swapchain(GPUSwapchain swapchain) override;
  [[nodiscard]] auto get_swapchain_back_buffer_index(GPUSwapchain swapchain)
      -> std::uint32_t override;
  void resize_swapchain(GPUSwapchain& swapchain, std::uint32_t width,
                        std::uint32_t height) override;

  [[nodiscard]] auto create_buffer(const BufferCreateInfo& create_info)
      -> Buffer override
  {
    panic("Unimplemented");
  }

  [[nodiscard]] auto
  create_compute_pipeline(const ComputePipelineCreateInfo& create_info)
      -> ComputePipeline override
  {
    panic("Unimplemented");
  }

  /**
   * @brief Destories the device buffer.
   *
   * This function destories the device buffer refered to by `buffer_handle` and
   * release its underlying memory. If the `buffer_handle` does not refer to a
   * valid device buffer, this function will do nothing.
   */
  auto destory_buffer(Buffer& buffer_handle) -> void override
  {
    panic("Unimplemented");
  }

  /// @brief Submits a sequence of command buffers to execute
  auto submit(gsl::span<SubmitInfo> infos) -> void override
  {
    panic("Unimplemented");
  }

  /**
   * @brief Maps the underlying memory of buffer to a pointer
   *
   * After a successful call to `map` buffer memory is
   * considered to be currently host mapped. If the `buffer` handle does not
   * refer to an real buffer, or if its underlying memory is not host visible,
   * this function will return `nullptr`
   */
  [[nodiscard]] virtual auto map(Buffer buffer) noexcept -> void*
  {
    panic("Unimplemented");
  }

  auto unmap(Buffer buffer) noexcept -> void
  {
    panic("Unimplemented");
  }

private:
  // Initialize your Graphics API
  void initializeAPI(Window& window);

  // Destroy any Graphics API data structures used in this example
  void destroyAPI();

  // Destroy any resources used in this example
  void destroyResources();

  // Create graphics API specific data structures to send commands to the GPU
  void createCommands();

  // Destroy all commands
  void destroyCommands();

  // Set up the FrameBuffer
  void init_frame_buffer(GPUSwapchain swapchain);

  void destroy_frame_buffer();

  // Set up the swapchain
  void setup_swapchain(GPUSwapchain& swapchain, unsigned width,
                       unsigned height);

  struct Vertex {
    float position[3];
    float color[3];
  };

  Vertex mVertexBufferData[3] = {{{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                                 {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                                 {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

  uint32_t mIndexBufferData[3] = {0, 1, 2};

  std::chrono::time_point<std::chrono::steady_clock> t_start, t_end;
  float elapsed_time_ = 0.0f;

  // Uniform data
  struct {
    glm::mat4 projectionMatrix;
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
  } ubo_data_;

  static const UINT backbuffer_count_ = 2;

  HWND window_{};
  unsigned width_ = 0, height_ = 0;

  // Initialization
  IDXGIFactory4* factory_ = nullptr;
  IDXGIAdapter1* adapter_ = nullptr;
#if defined(_DEBUG)
  ID3D12Debug1* debug_controller_ = nullptr;
  ID3D12DebugDevice* mDebugDevice = nullptr;
#endif
  ID3D12Device* device_ = nullptr;
  ID3D12CommandQueue* command_queue_ = nullptr;
  ID3D12CommandAllocator* command_allocator_ = nullptr;
  ID3D12GraphicsCommandList* command_list_ = nullptr;

  // Current Frame
  UINT mCurrentBuffer;
  ID3D12DescriptorHeap* mRtvHeap;
  ID3D12Resource* render_target_[backbuffer_count_] = {};
  // IDXGISwapChain3* swap_chain_ = nullptr;

  // Resources
  D3D12_VIEWPORT mViewport;
  D3D12_RECT mSurfaceSize;

  ID3D12Resource* vertex_buffer_ = nullptr;
  ID3D12Resource* index_buffer_ = nullptr;
  ID3D12Resource* uniform_buffer_ = nullptr;
  ID3D12DescriptorHeap* uniform_buffer_heap_ = nullptr;
  UINT8* mapped_uniform_buffer_ = nullptr;

  D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
  D3D12_INDEX_BUFFER_VIEW mIndexBufferView;

  UINT mRtvDescriptorSize;
  ID3D12RootSignature* mRootSignature;
  ID3D12PipelineState* pipeline_state_;

  // Sync
  UINT frame_index_ = 0;
  HANDLE fence_event_;
  ID3D12Fence* fence_;
  UINT64 fence_value_;
};

} // namespace beyond::graphics::d3d12

#endif // BEYOND_GRAPHICS_D3D12_GPU_DEVICE_HPP
