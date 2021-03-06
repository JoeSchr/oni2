let all = (join: ('a, 'a) => 'a, promises: list(Lwt.t('a))) => {
  List.fold_left(
    (accPromise, promise) => {
      let%lwt acc = accPromise;
      let%lwt curr = promise;
      Lwt.return(join(acc, curr));
    },
    Lwt.return([]),
    promises,
  );
};

let some = (~default: 'a, join: ('a, 'a) => 'a, promises: list(Lwt.t('a))) => {
  promises
  |> List.map(p => {
       try%lwt(p) {
       | _exn => Lwt.return(default)
       }
     })
  |> all(join);
};

exception Timeout;

let sync: (~timeout: float=?, Lwt.t('a)) => result('a, exn) =
  (~timeout=10.0, promise) => {
    let completed = ref(None);

    Lwt.on_success(promise, v => {completed := Some(Ok(v))});

    Lwt.on_failure(promise, v => {completed := Some(Error(v))});

    ThreadEx.waitForCondition(~timeout, () => {completed^ != None});

    Option.value(~default=Error(Timeout), completed^);
  };
