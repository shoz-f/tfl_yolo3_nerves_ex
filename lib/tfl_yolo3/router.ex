defmodule TflYolo3.Router do
  use Plug.Router
  
  alias TflYolo3.TflInterp

  @photo_file if Mix.target() == :host, do: Application.app_dir(:tfl_yolo3, "priv/photo.jpg"), else: "/root/photo.jpg" 
  @opening     Application.app_dir(:tfl_yolo3, "priv/photo.jpg")

  plug Plug.Static.IndexHtml, at: "/"
  plug Plug.Static, at: "/", from: :tfl_yolo3

  plug :match
  plug Plug.Parsers, parsers: [:urlencoded, :multipart, :json],
                     pass: ["text/*"],
                     json_decoder: Jason
  plug :dispatch

  get "/photo.jpg" do
    conn
    |> put_resp_header("content-type", "image/jpeg")
    |> send_file(200, if(File.exists?(@photo_file), do: @photo_file, else: @opening))
  end

  post "/predict_photo" do
    with \
      :ok <- File.cp(conn.params["photo"].path, @photo_file),
      {:ok, ans} <- TflInterp.predict(@photo_file)
    do
      IO.inspect(ans)
      send_resp(conn, 200, Jason.encode!(%{"ans" => ans}))
    else
      {:timeout} ->
        send_resp(conn, 200, "tiemout")
      {:error, error} ->
        send_resp(conn, 200, error)
      _ ->
        send_resp(conn, 200, "error")
    end
  end

  match _ do
    IO.inspect(conn)
    send_resp(conn, 404, "Oops!")
  end
end
