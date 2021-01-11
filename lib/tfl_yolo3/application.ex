defmodule TflYolo3.Application do
  # See https://hexdocs.pm/elixir/Application.html
  # for more information on OTP Applications
  @moduledoc false

  use Application

  def start(_type, _args) do
    children = [
        {Plug.Cowboy, scheme: :http, plug: TflYolo3.Router, options: [port: 5000]},
        {TflInterp, [model: "priv/yolov3-416-dr.tflite", label: "priv/coco.label", opts: "-n"]}
    ] ++ children(target())

    # See https://hexdocs.pm/elixir/Supervisor.html
    # for other strategies and supported options
    opts = [strategy: :one_for_one, name: TflYolo3.Supervisor]

    Supervisor.start_link(children, opts)
  end

  # List all child processes to be supervised
  def children(:host) do
    [
      # Children that only run on the host
      # Starts a worker by calling: TflYolo3.Worker.start_link(arg)
      # {TflYolo3.Worker, arg},
    ]
  end

  def children(_target) do
    [
      # Children for all targets except host
      # Starts a worker by calling: TflYolo3.Worker.start_link(arg)
      # {TflYolo3.Worker, arg},
    ]
  end

  def target() do
    Application.get_env(:tfl_yolo3, :target)
  end
end
