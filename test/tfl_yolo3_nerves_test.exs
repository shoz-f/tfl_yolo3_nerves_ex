defmodule TflYolo3NervesTest do
  use ExUnit.Case
  doctest TflYolo3Nerves

  test "object detection in 'dog.jpg'" do
    {:ok, res} = TflYolo3.TflInterp.predict("test/dog.jpg")
    assert Map.keys(res) == ["bicycle", "dog", "truck"]
    assert res["bicycle"] == [[164, 115, 567, 435]]
    assert res["dog"    ] == [[124, 224, 323, 544]]
    assert res["truck"  ] == [[472,  86, 692, 167]]
  end
  
  test "object detection in 'person.jpg'" do
    {:ok, res} = TflYolo3.TflInterp.predict("test/person.jpg")
    assert Map.keys(res) == ["dog", "horse", "person"]
    assert res["dog"   ] == [[ 62, 262, 204, 352]]
    assert res["horse" ] == [[396, 137, 613, 352]]
    assert res["person"] == [[190,  90, 277, 388]]
  end
end
