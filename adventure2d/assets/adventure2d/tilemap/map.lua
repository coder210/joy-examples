return {
  version = "1.10",
  luaversion = "5.1",
  tiledversion = "1.11.2",
  class = "",
  orientation = "orthogonal",
  renderorder = "right-down",
  width = 60,
  height = 40,
  tilewidth = 64,
  tileheight = 64,
  nextlayerid = 9,
  nextobjectid = 66,
  properties = {},
  tilesets = {
    {
      name = "tilemap_flat",
      firstgid = 1,
      class = "",
      tilewidth = 64,
      tileheight = 64,
      spacing = 0,
      margin = 0,
      columns = 10,
      image = "terrain/ground/tilemap_flat.png",
      imagewidth = 640,
      imageheight = 256,
      objectalignment = "unspecified",
      tilerendersize = "tile",
      fillmode = "stretch",
      tileoffset = {
        x = 0,
        y = 0
      },
      grid = {
        orientation = "orthogonal",
        width = 64,
        height = 64
      },
      properties = {},
      wangsets = {},
      tilecount = 40,
      tiles = {}
    },
    {
      name = "water",
      firstgid = 41,
      class = "",
      tilewidth = 64,
      tileheight = 64,
      spacing = 0,
      margin = 0,
      columns = 1,
      image = "terrain/water/water.png",
      imagewidth = 64,
      imageheight = 64,
      objectalignment = "unspecified",
      tilerendersize = "tile",
      fillmode = "stretch",
      tileoffset = {
        x = 0,
        y = 0
      },
      grid = {
        orientation = "orthogonal",
        width = 64,
        height = 64
      },
      properties = {},
      wangsets = {},
      tilecount = 1,
      tiles = {}
    },
    {
      name = "bridge_all",
      firstgid = 42,
      class = "",
      tilewidth = 64,
      tileheight = 64,
      spacing = 0,
      margin = 0,
      columns = 3,
      image = "terrain/bridge/bridge_all.png",
      imagewidth = 192,
      imageheight = 256,
      objectalignment = "unspecified",
      tilerendersize = "tile",
      fillmode = "stretch",
      tileoffset = {
        x = 0,
        y = 0
      },
      grid = {
        orientation = "orthogonal",
        width = 64,
        height = 64
      },
      properties = {},
      wangsets = {},
      tilecount = 12,
      tiles = {}
    },
    {
      name = "foam",
      firstgid = 54,
      class = "",
      tilewidth = 64,
      tileheight = 64,
      spacing = 0,
      margin = 0,
      columns = 24,
      image = "terrain/water/foam/foam.png",
      imagewidth = 1536,
      imageheight = 192,
      objectalignment = "unspecified",
      tilerendersize = "tile",
      fillmode = "stretch",
      tileoffset = {
        x = 0,
        y = 0
      },
      grid = {
        orientation = "orthogonal",
        width = 64,
        height = 64
      },
      properties = {},
      wangsets = {},
      tilecount = 72,
      tiles = {
        {
          id = 1,
          animation = {
            {
              tileid = 1,
              duration = 100
            },
            {
              tileid = 4,
              duration = 100
            },
            {
              tileid = 7,
              duration = 100
            },
            {
              tileid = 10,
              duration = 100
            },
            {
              tileid = 13,
              duration = 100
            },
            {
              tileid = 16,
              duration = 100
            },
            {
              tileid = 19,
              duration = 100
            },
            {
              tileid = 22,
              duration = 100
            }
          }
        },
        {
          id = 24,
          animation = {
            {
              tileid = 24,
              duration = 100
            },
            {
              tileid = 27,
              duration = 100
            },
            {
              tileid = 30,
              duration = 100
            },
            {
              tileid = 33,
              duration = 100
            },
            {
              tileid = 36,
              duration = 100
            },
            {
              tileid = 39,
              duration = 100
            },
            {
              tileid = 42,
              duration = 100
            },
            {
              tileid = 45,
              duration = 100
            }
          }
        },
        {
          id = 26,
          animation = {
            {
              tileid = 26,
              duration = 100
            },
            {
              tileid = 29,
              duration = 100
            },
            {
              tileid = 32,
              duration = 100
            },
            {
              tileid = 35,
              duration = 100
            },
            {
              tileid = 38,
              duration = 100
            },
            {
              tileid = 41,
              duration = 100
            },
            {
              tileid = 44,
              duration = 100
            },
            {
              tileid = 47,
              duration = 100
            }
          }
        },
        {
          id = 49,
          animation = {
            {
              tileid = 49,
              duration = 100
            },
            {
              tileid = 52,
              duration = 100
            },
            {
              tileid = 55,
              duration = 100
            },
            {
              tileid = 58,
              duration = 100
            },
            {
              tileid = 61,
              duration = 100
            },
            {
              tileid = 64,
              duration = 100
            },
            {
              tileid = 67,
              duration = 100
            },
            {
              tileid = 70,
              duration = 100
            }
          }
        }
      }
    },
    {
      name = "rocks_04",
      firstgid = 126,
      class = "",
      tilewidth = 128,
      tileheight = 128,
      spacing = 0,
      margin = 0,
      columns = 8,
      image = "terrain/water/rocks/rocks_04.png",
      imagewidth = 1024,
      imageheight = 128,
      objectalignment = "unspecified",
      tilerendersize = "tile",
      fillmode = "stretch",
      tileoffset = {
        x = 0,
        y = 0
      },
      grid = {
        orientation = "orthogonal",
        width = 128,
        height = 128
      },
      properties = {},
      wangsets = {},
      tilecount = 8,
      tiles = {
        {
          id = 0,
          animation = {
            {
              tileid = 0,
              duration = 100
            },
            {
              tileid = 1,
              duration = 100
            },
            {
              tileid = 2,
              duration = 100
            },
            {
              tileid = 3,
              duration = 100
            },
            {
              tileid = 4,
              duration = 100
            },
            {
              tileid = 5,
              duration = 100
            },
            {
              tileid = 6,
              duration = 100
            },
            {
              tileid = 7,
              duration = 100
            }
          }
        }
      }
    },
    {
      name = "happy_sheep_all",
      firstgid = 134,
      class = "",
      tilewidth = 128,
      tileheight = 128,
      spacing = 0,
      margin = 0,
      columns = 8,
      image = "textures/sheep/happy_sheep_all.png",
      imagewidth = 1024,
      imageheight = 256,
      objectalignment = "unspecified",
      tilerendersize = "tile",
      fillmode = "stretch",
      tileoffset = {
        x = 0,
        y = 0
      },
      grid = {
        orientation = "orthogonal",
        width = 128,
        height = 128
      },
      properties = {},
      wangsets = {},
      tilecount = 16,
      tiles = {
        {
          id = 0,
          animation = {
            {
              tileid = 0,
              duration = 100
            },
            {
              tileid = 1,
              duration = 100
            },
            {
              tileid = 2,
              duration = 100
            },
            {
              tileid = 3,
              duration = 100
            },
            {
              tileid = 4,
              duration = 100
            },
            {
              tileid = 5,
              duration = 100
            },
            {
              tileid = 6,
              duration = 100
            },
            {
              tileid = 7,
              duration = 100
            }
          }
        },
        {
          id = 8,
          animation = {
            {
              tileid = 8,
              duration = 100
            },
            {
              tileid = 9,
              duration = 100
            },
            {
              tileid = 10,
              duration = 100
            },
            {
              tileid = 11,
              duration = 100
            },
            {
              tileid = 12,
              duration = 100
            },
            {
              tileid = 13,
              duration = 100
            }
          }
        }
      }
    },
    {
      name = "tree",
      firstgid = 150,
      class = "",
      tilewidth = 192,
      tileheight = 192,
      spacing = 0,
      margin = 0,
      columns = 4,
      image = "textures/trees/tree.png",
      imagewidth = 768,
      imageheight = 576,
      objectalignment = "unspecified",
      tilerendersize = "tile",
      fillmode = "stretch",
      tileoffset = {
        x = 0,
        y = 0
      },
      grid = {
        orientation = "orthogonal",
        width = 192,
        height = 192
      },
      properties = {},
      wangsets = {},
      tilecount = 12,
      tiles = {
        {
          id = 0,
          animation = {
            {
              tileid = 0,
              duration = 100
            },
            {
              tileid = 1,
              duration = 100
            },
            {
              tileid = 2,
              duration = 100
            },
            {
              tileid = 3,
              duration = 100
            },
            {
              tileid = 4,
              duration = 100
            },
            {
              tileid = 5,
              duration = 100
            }
          }
        }
      }
    },
    {
      name = "fire",
      firstgid = 162,
      class = "",
      tilewidth = 128,
      tileheight = 128,
      spacing = 0,
      margin = 0,
      columns = 7,
      image = "effects/fire.png",
      imagewidth = 896,
      imageheight = 128,
      objectalignment = "unspecified",
      tilerendersize = "tile",
      fillmode = "stretch",
      tileoffset = {
        x = 0,
        y = 0
      },
      grid = {
        orientation = "orthogonal",
        width = 128,
        height = 128
      },
      properties = {},
      wangsets = {},
      tilecount = 7,
      tiles = {
        {
          id = 0,
          animation = {
            {
              tileid = 0,
              duration = 100
            },
            {
              tileid = 1,
              duration = 100
            },
            {
              tileid = 2,
              duration = 100
            },
            {
              tileid = 3,
              duration = 100
            },
            {
              tileid = 4,
              duration = 100
            },
            {
              tileid = 5,
              duration = 100
            },
            {
              tileid = 6,
              duration = 100
            }
          }
        }
      }
    },
    {
      name = "explosions",
      firstgid = 169,
      class = "",
      tilewidth = 192,
      tileheight = 192,
      spacing = 0,
      margin = 0,
      columns = 9,
      image = "effects/explosions.png",
      imagewidth = 1728,
      imageheight = 192,
      objectalignment = "unspecified",
      tilerendersize = "tile",
      fillmode = "stretch",
      tileoffset = {
        x = 0,
        y = 0
      },
      grid = {
        orientation = "orthogonal",
        width = 192,
        height = 192
      },
      properties = {},
      wangsets = {},
      tilecount = 9,
      tiles = {
        {
          id = 0,
          animation = {
            {
              tileid = 0,
              duration = 100
            },
            {
              tileid = 1,
              duration = 100
            },
            {
              tileid = 2,
              duration = 100
            },
            {
              tileid = 3,
              duration = 100
            },
            {
              tileid = 4,
              duration = 100
            },
            {
              tileid = 5,
              duration = 100
            },
            {
              tileid = 6,
              duration = 100
            },
            {
              tileid = 7,
              duration = 100
            },
            {
              tileid = 8,
              duration = 100
            }
          }
        }
      }
    },
    {
      name = "deco",
      firstgid = 178,
      class = "",
      tilewidth = 192,
      tileheight = 192,
      spacing = 0,
      margin = 0,
      columns = 0,
      objectalignment = "unspecified",
      tilerendersize = "tile",
      fillmode = "stretch",
      tileoffset = {
        x = 0,
        y = 0
      },
      grid = {
        orientation = "orthogonal",
        width = 1,
        height = 1
      },
      properties = {},
      wangsets = {},
      tilecount = 18,
      tiles = {
        {
          id = 0,
          image = "textures/deco/03.png",
          width = 64,
          height = 64
        },
        {
          id = 1,
          image = "textures/deco/04.png",
          width = 64,
          height = 64
        },
        {
          id = 2,
          image = "textures/deco/05.png",
          width = 64,
          height = 64
        },
        {
          id = 3,
          image = "textures/deco/06.png",
          width = 64,
          height = 64
        },
        {
          id = 4,
          image = "textures/deco/07.png",
          width = 64,
          height = 64
        },
        {
          id = 5,
          image = "textures/deco/08.png",
          width = 64,
          height = 64
        },
        {
          id = 6,
          image = "textures/deco/09.png",
          width = 64,
          height = 64
        },
        {
          id = 7,
          image = "textures/deco/10.png",
          width = 64,
          height = 64
        },
        {
          id = 8,
          image = "textures/deco/11.png",
          width = 64,
          height = 64
        },
        {
          id = 9,
          image = "textures/deco/12.png",
          width = 64,
          height = 64
        },
        {
          id = 10,
          image = "textures/deco/13.png",
          width = 64,
          height = 64
        },
        {
          id = 11,
          image = "textures/deco/14.png",
          width = 64,
          height = 64
        },
        {
          id = 12,
          image = "textures/deco/15.png",
          width = 64,
          height = 64
        },
        {
          id = 13,
          image = "textures/deco/16.png",
          width = 64,
          height = 128
        },
        {
          id = 14,
          image = "textures/deco/17.png",
          width = 64,
          height = 128
        },
        {
          id = 15,
          image = "textures/deco/18.png",
          width = 192,
          height = 192
        },
        {
          id = 16,
          image = "textures/deco/01.png",
          width = 64,
          height = 64
        },
        {
          id = 17,
          image = "textures/deco/02.png",
          width = 64,
          height = 64
        }
      }
    },
    {
      name = "warrior_blue",
      firstgid = 196,
      class = "",
      tilewidth = 192,
      tileheight = 192,
      spacing = 0,
      margin = 0,
      columns = 6,
      image = "textures/knights/troops/warrior/warrior_blue.png",
      imagewidth = 1152,
      imageheight = 1536,
      objectalignment = "unspecified",
      tilerendersize = "tile",
      fillmode = "stretch",
      tileoffset = {
        x = 0,
        y = 0
      },
      grid = {
        orientation = "orthogonal",
        width = 192,
        height = 192
      },
      properties = {},
      wangsets = {},
      tilecount = 48,
      tiles = {
        {
          id = 0,
          animation = {
            {
              tileid = 0,
              duration = 100
            },
            {
              tileid = 1,
              duration = 100
            },
            {
              tileid = 2,
              duration = 100
            },
            {
              tileid = 3,
              duration = 100
            },
            {
              tileid = 4,
              duration = 100
            },
            {
              tileid = 5,
              duration = 100
            }
          }
        },
        {
          id = 6,
          animation = {
            {
              tileid = 6,
              duration = 100
            },
            {
              tileid = 7,
              duration = 100
            },
            {
              tileid = 8,
              duration = 100
            },
            {
              tileid = 9,
              duration = 100
            },
            {
              tileid = 10,
              duration = 100
            },
            {
              tileid = 11,
              duration = 100
            }
          }
        },
        {
          id = 12,
          animation = {
            {
              tileid = 12,
              duration = 100
            },
            {
              tileid = 13,
              duration = 100
            },
            {
              tileid = 14,
              duration = 100
            },
            {
              tileid = 15,
              duration = 100
            },
            {
              tileid = 16,
              duration = 100
            },
            {
              tileid = 17,
              duration = 100
            }
          }
        },
        {
          id = 18,
          animation = {
            {
              tileid = 18,
              duration = 100
            },
            {
              tileid = 19,
              duration = 100
            },
            {
              tileid = 20,
              duration = 100
            },
            {
              tileid = 21,
              duration = 100
            },
            {
              tileid = 22,
              duration = 100
            },
            {
              tileid = 23,
              duration = 100
            }
          }
        },
        {
          id = 24,
          animation = {
            {
              tileid = 24,
              duration = 100
            },
            {
              tileid = 25,
              duration = 100
            },
            {
              tileid = 26,
              duration = 100
            },
            {
              tileid = 27,
              duration = 100
            },
            {
              tileid = 28,
              duration = 100
            },
            {
              tileid = 29,
              duration = 100
            }
          }
        },
        {
          id = 30,
          animation = {
            {
              tileid = 30,
              duration = 100
            },
            {
              tileid = 31,
              duration = 100
            },
            {
              tileid = 32,
              duration = 100
            },
            {
              tileid = 33,
              duration = 100
            },
            {
              tileid = 34,
              duration = 100
            },
            {
              tileid = 35,
              duration = 100
            }
          }
        },
        {
          id = 36,
          animation = {
            {
              tileid = 36,
              duration = 100
            },
            {
              tileid = 37,
              duration = 100
            },
            {
              tileid = 38,
              duration = 100
            },
            {
              tileid = 39,
              duration = 100
            },
            {
              tileid = 40,
              duration = 100
            },
            {
              tileid = 41,
              duration = 100
            }
          }
        },
        {
          id = 42,
          animation = {
            {
              tileid = 42,
              duration = 100
            },
            {
              tileid = 43,
              duration = 100
            },
            {
              tileid = 44,
              duration = 100
            },
            {
              tileid = 45,
              duration = 100
            },
            {
              tileid = 46,
              duration = 100
            },
            {
              tileid = 47,
              duration = 100
            }
          }
        }
      }
    },
    {
      name = "torch_red",
      firstgid = 244,
      class = "",
      tilewidth = 192,
      tileheight = 192,
      spacing = 0,
      margin = 0,
      columns = 7,
      image = "textures/goblins/troops/torch/torch_red.png",
      imagewidth = 1344,
      imageheight = 960,
      objectalignment = "unspecified",
      tilerendersize = "tile",
      fillmode = "stretch",
      tileoffset = {
        x = 0,
        y = 0
      },
      grid = {
        orientation = "orthogonal",
        width = 192,
        height = 192
      },
      properties = {},
      wangsets = {},
      tilecount = 35,
      tiles = {
        {
          id = 0,
          animation = {
            {
              tileid = 0,
              duration = 100
            },
            {
              tileid = 1,
              duration = 100
            },
            {
              tileid = 2,
              duration = 100
            },
            {
              tileid = 3,
              duration = 100
            },
            {
              tileid = 4,
              duration = 100
            },
            {
              tileid = 5,
              duration = 100
            },
            {
              tileid = 6,
              duration = 100
            }
          }
        },
        {
          id = 7,
          animation = {
            {
              tileid = 7,
              duration = 100
            },
            {
              tileid = 8,
              duration = 100
            },
            {
              tileid = 9,
              duration = 100
            },
            {
              tileid = 10,
              duration = 100
            },
            {
              tileid = 11,
              duration = 100
            },
            {
              tileid = 12,
              duration = 100
            }
          }
        },
        {
          id = 14,
          animation = {
            {
              tileid = 14,
              duration = 100
            },
            {
              tileid = 15,
              duration = 100
            },
            {
              tileid = 16,
              duration = 100
            },
            {
              tileid = 17,
              duration = 100
            },
            {
              tileid = 18,
              duration = 100
            },
            {
              tileid = 19,
              duration = 100
            }
          }
        },
        {
          id = 21,
          animation = {
            {
              tileid = 21,
              duration = 100
            },
            {
              tileid = 22,
              duration = 100
            },
            {
              tileid = 23,
              duration = 100
            },
            {
              tileid = 24,
              duration = 100
            },
            {
              tileid = 25,
              duration = 100
            },
            {
              tileid = 26,
              duration = 100
            }
          }
        },
        {
          id = 28,
          animation = {
            {
              tileid = 28,
              duration = 100
            },
            {
              tileid = 29,
              duration = 100
            },
            {
              tileid = 30,
              duration = 100
            },
            {
              tileid = 31,
              duration = 100
            },
            {
              tileid = 32,
              duration = 100
            },
            {
              tileid = 33,
              duration = 100
            }
          }
        }
      }
    },
    {
      name = "tnt_blue",
      firstgid = 279,
      class = "",
      tilewidth = 192,
      tileheight = 192,
      spacing = 0,
      margin = 0,
      columns = 7,
      image = "textures/goblins/troops/tnt/tnt_blue.png",
      imagewidth = 1344,
      imageheight = 576,
      objectalignment = "unspecified",
      tilerendersize = "tile",
      fillmode = "stretch",
      tileoffset = {
        x = 0,
        y = 0
      },
      grid = {
        orientation = "orthogonal",
        width = 192,
        height = 192
      },
      properties = {},
      wangsets = {},
      tilecount = 21,
      tiles = {
        {
          id = 0,
          animation = {
            {
              tileid = 0,
              duration = 100
            },
            {
              tileid = 1,
              duration = 100
            },
            {
              tileid = 2,
              duration = 100
            },
            {
              tileid = 3,
              duration = 100
            },
            {
              tileid = 4,
              duration = 100
            },
            {
              tileid = 5,
              duration = 100
            }
          }
        },
        {
          id = 7,
          animation = {
            {
              tileid = 7,
              duration = 100
            },
            {
              tileid = 8,
              duration = 100
            },
            {
              tileid = 9,
              duration = 100
            },
            {
              tileid = 10,
              duration = 100
            },
            {
              tileid = 11,
              duration = 100
            },
            {
              tileid = 12,
              duration = 100
            }
          }
        },
        {
          id = 14,
          animation = {
            {
              tileid = 14,
              duration = 100
            },
            {
              tileid = 15,
              duration = 100
            },
            {
              tileid = 16,
              duration = 100
            },
            {
              tileid = 17,
              duration = 100
            },
            {
              tileid = 18,
              duration = 100
            },
            {
              tileid = 19,
              duration = 100
            },
            {
              tileid = 20,
              duration = 100
            }
          }
        }
      }
    },
    {
      name = "dynamite",
      firstgid = 300,
      class = "",
      tilewidth = 64,
      tileheight = 64,
      spacing = 0,
      margin = 0,
      columns = 6,
      image = "textures/goblins/troops/tnt/dynamite.png",
      imagewidth = 384,
      imageheight = 64,
      objectalignment = "unspecified",
      tilerendersize = "tile",
      fillmode = "stretch",
      tileoffset = {
        x = 0,
        y = 0
      },
      grid = {
        orientation = "orthogonal",
        width = 64,
        height = 64
      },
      properties = {},
      wangsets = {},
      tilecount = 6,
      tiles = {
        {
          id = 0,
          animation = {
            {
              tileid = 0,
              duration = 100
            },
            {
              tileid = 1,
              duration = 100
            },
            {
              tileid = 2,
              duration = 100
            },
            {
              tileid = 3,
              duration = 100
            },
            {
              tileid = 4,
              duration = 100
            },
            {
              tileid = 5,
              duration = 100
            }
          }
        }
      }
    },
    {
      name = "tnt_purple",
      firstgid = 306,
      class = "",
      tilewidth = 192,
      tileheight = 192,
      spacing = 0,
      margin = 0,
      columns = 7,
      image = "textures/goblins/troops/tnt/tnt_purple.png",
      imagewidth = 1344,
      imageheight = 576,
      objectalignment = "unspecified",
      tilerendersize = "tile",
      fillmode = "stretch",
      tileoffset = {
        x = 0,
        y = 0
      },
      grid = {
        orientation = "orthogonal",
        width = 192,
        height = 192
      },
      properties = {},
      wangsets = {},
      tilecount = 21,
      tiles = {
        {
          id = 0,
          animation = {
            {
              tileid = 0,
              duration = 100
            },
            {
              tileid = 1,
              duration = 100
            },
            {
              tileid = 2,
              duration = 100
            },
            {
              tileid = 3,
              duration = 100
            },
            {
              tileid = 4,
              duration = 100
            },
            {
              tileid = 5,
              duration = 100
            }
          }
        },
        {
          id = 7,
          animation = {
            {
              tileid = 7,
              duration = 100
            },
            {
              tileid = 8,
              duration = 100
            },
            {
              tileid = 9,
              duration = 100
            },
            {
              tileid = 10,
              duration = 100
            },
            {
              tileid = 11,
              duration = 100
            },
            {
              tileid = 12,
              duration = 100
            }
          }
        },
        {
          id = 14,
          animation = {
            {
              tileid = 14,
              duration = 100
            },
            {
              tileid = 15,
              duration = 100
            },
            {
              tileid = 16,
              duration = 100
            },
            {
              tileid = 17,
              duration = 100
            },
            {
              tileid = 18,
              duration = 100
            },
            {
              tileid = 19,
              duration = 100
            },
            {
              tileid = 20,
              duration = 100
            }
          }
        }
      }
    },
    {
      name = "pawn_blue",
      firstgid = 327,
      class = "",
      tilewidth = 192,
      tileheight = 192,
      spacing = 0,
      margin = 0,
      columns = 6,
      image = "textures/knights/troops/pawn/pawn_blue.png",
      imagewidth = 1152,
      imageheight = 1152,
      objectalignment = "unspecified",
      tilerendersize = "tile",
      fillmode = "stretch",
      tileoffset = {
        x = 0,
        y = 0
      },
      grid = {
        orientation = "orthogonal",
        width = 192,
        height = 192
      },
      properties = {},
      wangsets = {},
      tilecount = 36,
      tiles = {
        {
          id = 0,
          animation = {
            {
              tileid = 0,
              duration = 100
            },
            {
              tileid = 1,
              duration = 100
            },
            {
              tileid = 2,
              duration = 100
            },
            {
              tileid = 3,
              duration = 100
            },
            {
              tileid = 4,
              duration = 100
            },
            {
              tileid = 5,
              duration = 100
            }
          }
        },
        {
          id = 6,
          animation = {
            {
              tileid = 6,
              duration = 100
            },
            {
              tileid = 7,
              duration = 100
            },
            {
              tileid = 8,
              duration = 100
            },
            {
              tileid = 9,
              duration = 100
            },
            {
              tileid = 10,
              duration = 100
            },
            {
              tileid = 11,
              duration = 100
            }
          }
        },
        {
          id = 12,
          animation = {
            {
              tileid = 12,
              duration = 100
            },
            {
              tileid = 13,
              duration = 100
            },
            {
              tileid = 14,
              duration = 100
            },
            {
              tileid = 15,
              duration = 100
            },
            {
              tileid = 16,
              duration = 100
            },
            {
              tileid = 17,
              duration = 100
            }
          }
        },
        {
          id = 18,
          animation = {
            {
              tileid = 18,
              duration = 100
            },
            {
              tileid = 19,
              duration = 100
            },
            {
              tileid = 20,
              duration = 100
            },
            {
              tileid = 21,
              duration = 100
            },
            {
              tileid = 22,
              duration = 100
            },
            {
              tileid = 23,
              duration = 100
            }
          }
        },
        {
          id = 24,
          animation = {
            {
              tileid = 24,
              duration = 100
            },
            {
              tileid = 25,
              duration = 100
            },
            {
              tileid = 26,
              duration = 100
            },
            {
              tileid = 27,
              duration = 100
            },
            {
              tileid = 28,
              duration = 100
            },
            {
              tileid = 29,
              duration = 100
            }
          }
        },
        {
          id = 30,
          animation = {
            {
              tileid = 30,
              duration = 100
            },
            {
              tileid = 31,
              duration = 100
            },
            {
              tileid = 32,
              duration = 100
            },
            {
              tileid = 33,
              duration = 100
            },
            {
              tileid = 34,
              duration = 100
            },
            {
              tileid = 35,
              duration = 100
            }
          }
        }
      }
    },
    {
      name = "barrel_blue",
      firstgid = 363,
      class = "",
      tilewidth = 128,
      tileheight = 128,
      spacing = 0,
      margin = 0,
      columns = 6,
      image = "textures/goblins/troops/barrel/barrel_blue.png",
      imagewidth = 768,
      imageheight = 768,
      objectalignment = "unspecified",
      tilerendersize = "tile",
      fillmode = "stretch",
      tileoffset = {
        x = 0,
        y = 0
      },
      grid = {
        orientation = "orthogonal",
        width = 128,
        height = 128
      },
      properties = {},
      wangsets = {},
      tilecount = 36,
      tiles = {
        {
          id = 0,
          animation = {
            {
              tileid = 0,
              duration = 100
            },
            {
              tileid = 6,
              duration = 100
            },
            {
              tileid = 7,
              duration = 100
            },
            {
              tileid = 8,
              duration = 100
            },
            {
              tileid = 9,
              duration = 100
            },
            {
              tileid = 10,
              duration = 100
            },
            {
              tileid = 11,
              duration = 100
            }
          }
        },
        {
          id = 12,
          animation = {
            {
              tileid = 12,
              duration = 100
            },
            {
              tileid = 18,
              duration = 100
            },
            {
              tileid = 19,
              duration = 100
            },
            {
              tileid = 20,
              duration = 100
            },
            {
              tileid = 21,
              duration = 100
            },
            {
              tileid = 22,
              duration = 100
            },
            {
              tileid = 23,
              duration = 100
            }
          }
        },
        {
          id = 18,
          animation = {
            {
              tileid = 24,
              duration = 100
            },
            {
              tileid = 25,
              duration = 100
            },
            {
              tileid = 26,
              duration = 100
            },
            {
              tileid = 32,
              duration = 100
            },
            {
              tileid = 31,
              duration = 100
            },
            {
              tileid = 30,
              duration = 100
            }
          }
        }
      }
    },
    {
      name = "archer_blue",
      firstgid = 399,
      class = "",
      tilewidth = 192,
      tileheight = 192,
      spacing = 0,
      margin = 0,
      columns = 8,
      image = "textures/knights/troops/archer/archer_blue.png",
      imagewidth = 1536,
      imageheight = 1344,
      objectalignment = "unspecified",
      tilerendersize = "tile",
      fillmode = "stretch",
      tileoffset = {
        x = 0,
        y = 0
      },
      grid = {
        orientation = "orthogonal",
        width = 192,
        height = 192
      },
      properties = {},
      wangsets = {},
      tilecount = 56,
      tiles = {
        {
          id = 0,
          animation = {
            {
              tileid = 0,
              duration = 100
            },
            {
              tileid = 1,
              duration = 100
            },
            {
              tileid = 2,
              duration = 100
            },
            {
              tileid = 3,
              duration = 100
            },
            {
              tileid = 4,
              duration = 100
            },
            {
              tileid = 5,
              duration = 100
            }
          }
        },
        {
          id = 8,
          animation = {
            {
              tileid = 8,
              duration = 100
            },
            {
              tileid = 9,
              duration = 100
            },
            {
              tileid = 10,
              duration = 100
            },
            {
              tileid = 11,
              duration = 100
            },
            {
              tileid = 12,
              duration = 100
            },
            {
              tileid = 13,
              duration = 100
            }
          }
        },
        {
          id = 16,
          animation = {
            {
              tileid = 16,
              duration = 100
            },
            {
              tileid = 17,
              duration = 100
            },
            {
              tileid = 18,
              duration = 100
            },
            {
              tileid = 19,
              duration = 100
            },
            {
              tileid = 20,
              duration = 100
            },
            {
              tileid = 21,
              duration = 100
            },
            {
              tileid = 22,
              duration = 100
            },
            {
              tileid = 23,
              duration = 100
            }
          }
        },
        {
          id = 24,
          animation = {
            {
              tileid = 24,
              duration = 100
            },
            {
              tileid = 25,
              duration = 100
            },
            {
              tileid = 26,
              duration = 100
            },
            {
              tileid = 27,
              duration = 100
            },
            {
              tileid = 28,
              duration = 100
            },
            {
              tileid = 29,
              duration = 100
            },
            {
              tileid = 30,
              duration = 100
            },
            {
              tileid = 31,
              duration = 100
            }
          }
        },
        {
          id = 32,
          animation = {
            {
              tileid = 32,
              duration = 100
            },
            {
              tileid = 33,
              duration = 100
            },
            {
              tileid = 34,
              duration = 100
            },
            {
              tileid = 35,
              duration = 100
            },
            {
              tileid = 36,
              duration = 100
            },
            {
              tileid = 37,
              duration = 100
            },
            {
              tileid = 38,
              duration = 100
            },
            {
              tileid = 39,
              duration = 100
            }
          }
        },
        {
          id = 40,
          animation = {
            {
              tileid = 40,
              duration = 100
            },
            {
              tileid = 41,
              duration = 100
            },
            {
              tileid = 42,
              duration = 100
            },
            {
              tileid = 43,
              duration = 100
            },
            {
              tileid = 44,
              duration = 100
            },
            {
              tileid = 45,
              duration = 100
            },
            {
              tileid = 46,
              duration = 100
            },
            {
              tileid = 47,
              duration = 100
            }
          }
        },
        {
          id = 48,
          animation = {
            {
              tileid = 48,
              duration = 100
            },
            {
              tileid = 49,
              duration = 100
            },
            {
              tileid = 50,
              duration = 100
            },
            {
              tileid = 51,
              duration = 100
            },
            {
              tileid = 52,
              duration = 100
            },
            {
              tileid = 53,
              duration = 100
            },
            {
              tileid = 54,
              duration = 100
            },
            {
              tileid = 55,
              duration = 100
            }
          }
        }
      }
    },
    {
      name = "archer_blue",
      firstgid = 455,
      class = "",
      tilewidth = 192,
      tileheight = 192,
      spacing = 0,
      margin = 0,
      columns = 8,
      image = "textures/knights/troops/archer/archer_blue.png",
      imagewidth = 1536,
      imageheight = 1344,
      objectalignment = "unspecified",
      tilerendersize = "tile",
      fillmode = "stretch",
      tileoffset = {
        x = 0,
        y = 0
      },
      grid = {
        orientation = "orthogonal",
        width = 192,
        height = 192
      },
      properties = {},
      wangsets = {},
      tilecount = 56,
      tiles = {
        {
          id = 0,
          animation = {
            {
              tileid = 0,
              duration = 100
            },
            {
              tileid = 1,
              duration = 100
            },
            {
              tileid = 2,
              duration = 100
            },
            {
              tileid = 3,
              duration = 100
            },
            {
              tileid = 4,
              duration = 100
            },
            {
              tileid = 5,
              duration = 100
            }
          }
        },
        {
          id = 8,
          animation = {
            {
              tileid = 8,
              duration = 100
            },
            {
              tileid = 9,
              duration = 100
            },
            {
              tileid = 10,
              duration = 100
            },
            {
              tileid = 11,
              duration = 100
            },
            {
              tileid = 12,
              duration = 100
            },
            {
              tileid = 13,
              duration = 100
            }
          }
        },
        {
          id = 16,
          animation = {
            {
              tileid = 16,
              duration = 100
            },
            {
              tileid = 17,
              duration = 100
            },
            {
              tileid = 18,
              duration = 100
            },
            {
              tileid = 19,
              duration = 100
            },
            {
              tileid = 20,
              duration = 100
            },
            {
              tileid = 21,
              duration = 100
            },
            {
              tileid = 22,
              duration = 100
            },
            {
              tileid = 23,
              duration = 100
            }
          }
        },
        {
          id = 24,
          animation = {
            {
              tileid = 24,
              duration = 100
            },
            {
              tileid = 25,
              duration = 100
            },
            {
              tileid = 26,
              duration = 100
            },
            {
              tileid = 27,
              duration = 100
            },
            {
              tileid = 28,
              duration = 100
            },
            {
              tileid = 29,
              duration = 100
            },
            {
              tileid = 30,
              duration = 100
            },
            {
              tileid = 31,
              duration = 100
            }
          }
        },
        {
          id = 32,
          animation = {
            {
              tileid = 32,
              duration = 100
            },
            {
              tileid = 33,
              duration = 100
            },
            {
              tileid = 34,
              duration = 100
            },
            {
              tileid = 35,
              duration = 100
            },
            {
              tileid = 36,
              duration = 100
            },
            {
              tileid = 37,
              duration = 100
            },
            {
              tileid = 38,
              duration = 100
            },
            {
              tileid = 39,
              duration = 100
            }
          }
        },
        {
          id = 40,
          animation = {
            {
              tileid = 40,
              duration = 100
            },
            {
              tileid = 41,
              duration = 100
            },
            {
              tileid = 42,
              duration = 100
            },
            {
              tileid = 43,
              duration = 100
            },
            {
              tileid = 44,
              duration = 100
            },
            {
              tileid = 45,
              duration = 100
            },
            {
              tileid = 46,
              duration = 100
            },
            {
              tileid = 47,
              duration = 100
            }
          }
        },
        {
          id = 48,
          animation = {
            {
              tileid = 48,
              duration = 100
            },
            {
              tileid = 49,
              duration = 100
            },
            {
              tileid = 50,
              duration = 100
            },
            {
              tileid = 51,
              duration = 100
            },
            {
              tileid = 52,
              duration = 100
            },
            {
              tileid = 53,
              duration = 100
            },
            {
              tileid = 54,
              duration = 100
            },
            {
              tileid = 55,
              duration = 100
            }
          }
        }
      }
    }
  },
  layers = {
    {
      type = "objectgroup",
      draworder = "topdown",
      id = 6,
      name = "collision",
      class = "",
      visible = true,
      opacity = 1,
      offsetx = 0,
      offsety = 0,
      parallaxx = 1,
      parallaxy = 1,
      tintcolor = { 255, 85, 0 },
      properties = {},
      objects = {
        {
          id = 13,
          name = "l_air_wall",
          type = "",
          shape = "rectangle",
          x = 0,
          y = 0,
          width = 120,
          height = 2560,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 14,
          name = "t_air_wall",
          type = "",
          shape = "rectangle",
          x = 0,
          y = 0,
          width = 3840,
          height = 128,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 15,
          name = "r_air_wall",
          type = "",
          shape = "rectangle",
          x = 3712,
          y = 0,
          width = 128,
          height = 2560,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 16,
          name = "b_air_wall",
          type = "",
          shape = "rectangle",
          x = 0,
          y = 2432,
          width = 3840,
          height = 128,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 25,
          name = "wall_1",
          type = "",
          shape = "rectangle",
          x = 3136,
          y = 250,
          width = 448,
          height = 70,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 27,
          name = "wall_2",
          type = "",
          shape = "rectangle",
          x = 3134,
          y = 382,
          width = 446,
          height = 256,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 29,
          name = "wall_3",
          type = "",
          shape = "rectangle",
          x = 3458,
          y = 640,
          width = 120,
          height = 256,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 32,
          name = "wall_4",
          type = "",
          shape = "rectangle",
          x = 3138,
          y = 640,
          width = 122,
          height = 384,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 35,
          name = "wall_5",
          type = "",
          shape = "rectangle",
          x = 3578,
          y = 1404,
          width = 134,
          height = 260,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 41,
          name = "wall_6",
          type = "",
          shape = "rectangle",
          x = 2942,
          y = 1404,
          width = 446,
          height = 258,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 44,
          name = "wall_7",
          type = "",
          shape = "rectangle",
          x = 2686,
          y = 136,
          width = 256,
          height = 566,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 45,
          name = "wall_8",
          type = "",
          shape = "rectangle",
          x = 2816,
          y = 704,
          width = 128,
          height = 444,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 46,
          name = "wall_9",
          type = "",
          shape = "rectangle",
          x = 2816,
          y = 1218,
          width = 124,
          height = 1020,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 47,
          name = "wall_10",
          type = "",
          shape = "rectangle",
          x = 2812,
          y = 2300,
          width = 130,
          height = 128,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 48,
          name = "wall_11",
          type = "",
          shape = "rectangle",
          x = 2234,
          y = 572,
          width = 448,
          height = 132,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 49,
          name = "wall_12",
          type = "",
          shape = "rectangle",
          x = 1984,
          y = 1852,
          width = 832,
          height = 128,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 51,
          name = "wall_13",
          type = "",
          shape = "rectangle",
          x = 1088,
          y = 574,
          width = 1090,
          height = 130,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 52,
          name = "wall_14",
          type = "",
          shape = "rectangle",
          x = 1088,
          y = 1852,
          width = 832,
          height = 128,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 53,
          name = "wall_15",
          type = "",
          shape = "rectangle",
          x = 1084,
          y = 700,
          width = 124,
          height = 644,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 54,
          name = "wall_16",
          type = "",
          shape = "rectangle",
          x = 1088,
          y = 1412,
          width = 124,
          height = 568,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 55,
          name = "wall_17",
          type = "",
          shape = "rectangle",
          x = 1152,
          y = 1979,
          width = 128,
          height = 322,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 56,
          name = "wall_18",
          type = "",
          shape = "rectangle",
          x = 1152,
          y = 2370,
          width = 128,
          height = 62,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 57,
          name = "wall_19",
          type = "",
          shape = "rectangle",
          x = 573,
          y = 703,
          width = 512,
          height = 255,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 58,
          name = "wall_20",
          type = "",
          shape = "rectangle",
          x = 709,
          y = 1082,
          width = 179,
          height = 706,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 59,
          name = "wall_21",
          type = "",
          shape = "rectangle",
          x = 576,
          y = 1221,
          width = 128,
          height = 564,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 60,
          name = "wall_22",
          type = "",
          shape = "rectangle",
          x = 385,
          y = 1209,
          width = 127,
          height = 458,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 61,
          name = "wall_23",
          type = "",
          shape = "rectangle",
          x = 252,
          y = 1216,
          width = 132,
          height = 248,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 62,
          name = "wall_24",
          type = "",
          shape = "rectangle",
          x = 127,
          y = 697,
          width = 255,
          height = 261,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 63,
          name = "wall_25",
          type = "",
          shape = "rectangle",
          x = 127,
          y = 1982,
          width = 1025,
          height = 127,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 64,
          name = "wall_26",
          type = "",
          shape = "rectangle",
          x = 961,
          y = 312,
          width = 130,
          height = 392,
          rotation = 0,
          visible = true,
          properties = {}
        },
        {
          id = 65,
          name = "wall_27",
          type = "",
          shape = "rectangle",
          x = 956,
          y = 126,
          width = 132,
          height = 122,
          rotation = 0,
          visible = true,
          properties = {}
        }
      }
    },
    {
      type = "imagelayer",
      image = "terrain/water/water.png",
      id = 2,
      name = "background",
      class = "",
      visible = false,
      opacity = 1,
      offsetx = 0,
      offsety = 0,
      parallaxx = 1,
      parallaxy = 1,
      repeatx = true,
      repeaty = true,
      properties = {}
    },
    {
      type = "tilelayer",
      x = 0,
      y = 0,
      width = 60,
      height = 40,
      id = 1,
      name = "terrain",
      class = "",
      visible = true,
      opacity = 1,
      offsetx = 0,
      offsety = 0,
      parallaxx = 1,
      parallaxy = 1,
      properties = {},
      encoding = "lua",
      data = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 0, 0, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 0, 0, 0, 0, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 0, 0,
        0, 78, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 80, 78, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 80, 0, 0, 78, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 80, 0,
        0, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 55, 55, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0, 0, 78, 11, 12, 12, 22, 22, 22, 22, 22, 22, 22, 12, 13, 80, 0,
        0, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 42, 43, 43, 44, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0, 0, 78, 11, 12, 13, 55, 55, 55, 55, 55, 55, 55, 11, 13, 80, 0,
        0, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 103, 103, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0, 0, 78, 11, 12, 13, 32, 32, 32, 32, 32, 32, 32, 12, 13, 80, 0,
        0, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0, 0, 78, 11, 12, 13, 80, 103, 103, 103, 103, 103, 103, 11, 13, 80, 0,
        0, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0, 0, 78, 11, 12, 13, 80, 0, 126, 0, 0, 0, 78, 11, 13, 80, 0,
        0, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 78, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 45, 22, 22, 22, 22, 22, 22, 23, 80, 0, 0, 78, 11, 12, 13, 80, 126, 0, 126, 0, 0, 78, 11, 13, 80, 0,
        0, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 78, 48, 80, 103, 103, 103, 103, 103, 103, 0, 0, 0, 78, 11, 12, 13, 80, 0, 55, 55, 55, 0, 78, 11, 13, 80, 0,
        0, 78, 21, 22, 22, 22, 12, 12, 12, 22, 22, 22, 22, 22, 23, 80, 0, 0, 0, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 78, 48, 80, 55, 55, 55, 55, 55, 55, 55, 55, 0, 78, 11, 12, 13, 80, 78, 1, 2, 3, 80, 78, 11, 13, 80, 0,
        0, 0, 103, 103, 103, 103, 11, 12, 13, 80, 103, 103, 103, 103, 103, 0, 0, 0, 78, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 51, 2, 2, 2, 2, 2, 2, 2, 2, 3, 80, 78, 11, 12, 13, 80, 78, 11, 12, 13, 80, 78, 11, 13, 80, 0,
        0, 0, 0, 0, 0, 78, 11, 12, 13, 80, 0, 0, 0, 0, 0, 0, 0, 0, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 78, 11, 12, 13, 80, 78, 11, 12, 13, 80, 78, 11, 13, 80, 0,
        0, 0, 0, 0, 0, 78, 11, 12, 13, 80, 0, 0, 0, 0, 0, 0, 0, 0, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 78, 11, 12, 13, 80, 78, 11, 12, 13, 80, 55, 11, 13, 80, 0,
        0, 0, 55, 55, 55, 55, 11, 12, 13, 55, 55, 55, 55, 55, 55, 55, 55, 0, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 78, 11, 12, 13, 80, 78, 11, 12, 12, 2, 2, 12, 13, 80, 0,
        0, 78, 1, 2, 2, 2, 12, 12, 12, 2, 2, 2, 2, 2, 2, 2, 3, 80, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 78, 11, 12, 13, 80, 78, 11, 12, 12, 12, 12, 12, 13, 80, 0,
        0, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 22, 22, 22, 12, 12, 13, 80, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 78, 11, 12, 12, 2, 2, 12, 12, 12, 12, 12, 12, 13, 80, 0,
        0, 78, 11, 12, 12, 12, 12, 12, 12, 12, 13, 103, 103, 103, 11, 12, 13, 80, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 55, 55, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0,
        0, 78, 11, 12, 22, 22, 22, 22, 12, 22, 23, 80, 0, 78, 11, 12, 13, 80, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 42, 43, 43, 44, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0,
        0, 78, 11, 13, 80, 103, 103, 103, 14, 80, 103, 0, 0, 78, 11, 12, 13, 80, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 103, 103, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0,
        0, 78, 11, 13, 80, 0, 0, 78, 14, 80, 0, 0, 0, 78, 11, 12, 13, 55, 55, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0,
        0, 78, 11, 13, 80, 0, 0, 78, 14, 80, 0, 0, 0, 78, 11, 12, 42, 43, 43, 44, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 78, 21, 22, 22, 22, 22, 22, 22, 12, 12, 12, 22, 23, 80, 0,
        0, 78, 11, 13, 55, 55, 0, 78, 14, 80, 0, 0, 0, 78, 11, 12, 13, 103, 103, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0, 103, 103, 103, 103, 103, 103, 78, 11, 12, 13, 80, 103, 0, 0,
        0, 78, 11, 12, 2, 3, 80, 78, 14, 80, 0, 0, 0, 78, 11, 12, 13, 80, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0, 0, 0, 0, 0, 0, 0, 78, 11, 12, 13, 80, 0, 0, 0,
        0, 78, 11, 12, 12, 13, 80, 78, 14, 80, 0, 0, 0, 78, 11, 12, 13, 80, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0, 0, 0, 0, 0, 0, 0, 78, 11, 12, 13, 80, 0, 0, 0,
        0, 78, 11, 12, 12, 13, 55, 55, 14, 80, 0, 0, 0, 78, 11, 12, 13, 80, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0, 55, 55, 55, 55, 55, 55, 55, 11, 12, 13, 80, 55, 0, 0,
        0, 78, 11, 12, 12, 12, 2, 2, 13, 80, 0, 0, 0, 78, 11, 12, 13, 80, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 78, 1, 2, 2, 2, 2, 2, 2, 12, 12, 12, 2, 3, 80, 0,
        0, 78, 11, 12, 12, 12, 12, 12, 13, 55, 55, 55, 55, 55, 11, 12, 13, 80, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0,
        0, 78, 11, 12, 12, 12, 12, 12, 12, 2, 2, 2, 2, 2, 12, 12, 13, 80, 78, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 45, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 80, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0,
        0, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 48, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 0, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0,
        0, 78, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 80, 0, 0, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 48, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 0, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0,
        0, 0, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 0, 0, 78, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 51, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 80, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0,
        0, 0, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 0, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0,
        0, 78, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 80, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0,
        0, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 55, 55, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0,
        0, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 55, 55, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 42, 43, 43, 44, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0,
        0, 78, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 42, 43, 43, 44, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 103, 103, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 80, 0,
        0, 78, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 103, 103, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 80, 78, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 80, 0,
        0, 0, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 0, 0, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 0, 0, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      }
    },
    {
      type = "tilelayer",
      x = 0,
      y = 0,
      width = 60,
      height = 40,
      id = 8,
      name = "foreground",
      class = "",
      visible = true,
      opacity = 1,
      offsetx = 0,
      offsety = 0,
      parallaxx = 1,
      parallaxy = 1,
      properties = {},
      encoding = "lua",
      data = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 186, 0, 0, 0, 0, 0, 0, 0, 0, 0, 181, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 181, 0, 188, 181, 0, 0, 186, 183, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 183, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 184, 0, 0, 187, 0, 0, 0, 184, 0, 0, 187, 0, 0, 0, 0, 181, 0, 0, 181, 0, 0, 0, 0, 0, 0, 0, 0, 0, 190, 0, 0, 0, 189, 0, 0, 0, 185, 0, 0, 0,
        0, 0, 0, 185, 0, 0, 0, 0, 0, 0, 0, 0, 183, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 187, 0, 0, 0, 0, 0, 0, 0, 186, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 189, 189, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 183, 0, 0, 0, 0, 150, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 180, 0, 0, 0, 185, 0, 0, 0, 0, 0, 0, 0, 0, 180, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 184, 0, 0, 0, 188, 183, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 183, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 181, 0, 0, 0, 0, 0, 0, 0, 0, 0, 185, 0, 0, 183, 0, 0, 0, 0, 187, 0, 0, 0, 0, 0, 0, 0, 0, 0, 187, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 181, 0, 0, 182, 0, 0, 0, 0, 0, 0, 0, 0, 0, 186, 0, 0, 0, 0, 185, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 190, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 186, 0, 0, 0, 0, 0, 0, 182, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 183, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 181, 0, 0, 0, 183, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 187, 0, 0, 150, 0, 150, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 150, 0, 0, 0, 0, 0, 182, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 184, 0, 181, 182, 0, 0, 0, 150, 0, 0, 0, 0, 0, 0, 182, 0, 0, 0, 0, 0, 0, 0, 190, 190, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 183, 0, 0, 0, 0, 0, 0, 0, 195, 0, 0, 0, 0, 0, 187, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 187, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 188, 0, 0, 0, 0, 0, 150, 0, 0, 0, 0, 0, 195, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 183, 189, 0, 0, 0, 0, 190, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 184, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 182, 0, 0, 0, 0, 185, 150, 0, 0, 195, 0, 0, 187, 0, 134, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 188, 0, 190, 0, 0,
        0, 0, 0, 0, 184, 0, 181, 0, 0, 183, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 182, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 192, 0, 0, 0, 0, 182, 0, 186, 0, 182, 0, 189, 0, 0, 0, 0, 0,
        0, 0, 183, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 188, 0, 0, 0, 0, 0, 0, 188, 0, 0, 0, 0, 0, 150, 0, 0, 0, 150, 0, 0, 0, 0, 0, 0, 0, 142, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 184, 0, 0, 0,
        0, 0, 185, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 182, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 188, 0, 0, 188, 0, 0, 0, 0, 0, 0, 0, 0, 0, 180, 0, 0, 187, 0, 0, 0, 0, 184, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 182, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 190, 180, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 192, 0, 0, 0, 0, 0, 0, 0, 0, 279, 0, 286, 0, 293, 0, 0, 0, 0, 0, 188, 220, 0, 0, 0, 0, 0, 0, 0, 0, 0, 186, 187, 0, 0, 0, 181, 0, 183, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 184, 0, 0, 0, 0, 0, 0, 327, 0, 333, 0, 339, 0, 0, 162, 0, 0, 0, 0, 0, 0, 196, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 357, 351, 300, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 208, 0, 188, 226, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 187, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 345, 0, 0, 0, 169, 0, 0, 0, 0, 0, 0, 0, 0, 202, 0, 0, 195, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 180, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 184, 0, 0, 0, 0, 0, 0, 0, 188, 0, 0, 0, 0, 0, 306, 0, 0, 0, 238, 0, 0, 194, 214, 0, 0, 232, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 182, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 188, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 187, 0, 0, 0, 0, 0, 0, 0, 0, 244, 0, 0, 251, 0, 185, 258, 0, 0, 0, 0, 0, 0, 0, 0, 0, 181, 188, 0, 0, 0, 0, 190, 0, 0, 0,
        0, 0, 0, 0, 180, 0, 0, 186, 0, 0, 0, 0, 0, 0, 0, 185, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 265, 185, 272, 0, 0, 180, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 189, 0, 181, 0, 0, 0, 180, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 187, 0, 187, 0, 0, 185, 0, 0, 0, 0, 0, 181, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 187, 0, 150, 0, 0, 0, 0, 190, 150, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 182, 186, 0, 0, 190, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 180, 0, 0, 150, 0, 0, 0, 0, 181, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 185, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 181, 0, 0, 0, 0, 0, 0, 187, 0, 0, 0, 0, 188, 0, 180, 0, 0, 0, 0, 0, 182, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 182, 0, 0, 0, 0, 0, 0, 150, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 184, 182, 0, 0, 0, 0, 0,
        0, 0, 0, 184, 0, 180, 0, 150, 0, 0, 150, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 187, 0, 0, 184, 150, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 180, 0, 0, 0, 183, 0, 0, 0, 0, 0, 0, 0, 0, 183, 0, 0, 0, 0, 0, 187, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 150, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 150, 0, 0, 0, 0, 0, 0, 185, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 188, 0, 186, 0, 0, 0, 0,
        0, 0, 0, 180, 181, 0, 187, 0, 182, 0, 150, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 182, 0, 0, 185, 187, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 187, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 183, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 188, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      }
    }
  }
}
