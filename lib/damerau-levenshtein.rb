# frozen_string_literal: true

require_relative "damerau-levenshtein/version"
require_relative "damerau-levenshtein/damerau_levenshtein"
require_relative "damerau-levenshtein/formatter"
require_relative "damerau-levenshtein/differ"

# Damerau-Levenshtein algorithm
module DamerauLevenshtein
  extend DamerauLevenshteinBinding

  def self.version
    VERSION
  end

  def self.distance(str1, str2, block_size = 1, max_distance = 10, cost: nil)
    internal_distance(
      str1.unpack("U*"), str2.unpack("U*"),
      block_size, max_distance, cost
    )
  end

  def self.string_distance(*args)
    distance(*args)
  end

  def self.array_distance(array1, array2, block_size = 1, max_distance = 10)
    internal_distance(array1, array2, block_size, max_distance)
  end

  # keep backward compatibility - internal_distance was called distance_utf
  # before
  def self.distance_utf(*args)
    internal_distance(*args)
  end

  class CustomCost
    def initialize
      @special = {}
    end

    def add_char(a, b, value)
      pair = (a+b).unpack('U*').sort
      @special[pair] = value
    end

    def add_char_set(chars, value)
      chars.combination(2) do |a, b|
        add_char(a, b, value)
      end
    end

    def call(a, b)
      pair = [a, b].sort
      @special.fetch(pair) do
        (a == b) ? 0.0 : 1.0
      end
    end
  end
end
