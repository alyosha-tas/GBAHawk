using System;
using System.Collections.Generic;
using System.Linq;

namespace BizHawk.Common
{
	public sealed class RigidMultiPredicateSort<T>
	{
		private readonly IReadOnlyDictionary<string, Func<T, IComparable>> _predicates;

		public RigidMultiPredicateSort(IReadOnlyDictionary<string, Func<T, IComparable>> predicates)
		{
			if (predicates.Count == 0) throw new ArgumentException("must have at least 1 predicate", nameof(predicates));
			_predicates = predicates;
		}

		/// <remarks>sorts using <paramref name="idOfFirst"/> asc/desc (by <paramref name="firstIsDesc"/>), then by the remaining predicates, all asc</remarks>
		public List<T> AppliedTo(IReadOnlyCollection<T> list, string idOfFirst, bool firstIsDesc = false)
		{
			var temp = firstIsDesc
				? list.OrderByDescending(_predicates[idOfFirst])
				: list.OrderBy(_predicates[idOfFirst]);
			foreach (var (id, pred) in _predicates)
			{
				if (id == idOfFirst) continue;
				temp = temp.ThenBy(pred);
			}
			return temp.ToList();
		}

		public List<T> AppliedTo(IReadOnlyCollection<T> list, string idOfFirst, IReadOnlyDictionary<string, bool> isDescMap)
		{
			var temp = isDescMap[idOfFirst]
				? list.OrderByDescending(_predicates[idOfFirst])
				: list.OrderBy(_predicates[idOfFirst]);
			foreach (var (id, pred) in _predicates)
			{
				if (id == idOfFirst) continue;
				temp = isDescMap[id] ? temp.ThenByDescending(pred) : temp.ThenBy(pred);
			}
			return temp.ToList();
		}
	}
}
