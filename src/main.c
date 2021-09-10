#include <emacs-module.h>
#include <stdint.h>

#include "get_process_cwd.h"

/* Declare mandatory GPL symbol.  */
int plugin_is_GPL_compatible;

static CHAR* narrow(const WCHAR* ws, const size_t wsLength, size_t* length) {
  // TODO error checking?
  const size_t bufLength = WideCharToMultiByte(CP_UTF8, 0, ws, wsLength, NULL, 0, NULL, NULL);
  CHAR *s = (CHAR*) malloc(bufLength + 1);
  WideCharToMultiByte(CP_UTF8, 0, ws, wsLength, s, bufLength, NULL, NULL);
  s[bufLength] = '\0';
  *length = bufLength;
  return s;
}

/* New emacs lisp function. All function exposed to Emacs must have this prototype. */
static emacs_value Fget_cwd_by_pid (emacs_env *env, ptrdiff_t nargs, emacs_value args[], void *data) {
  // TODO error checking
  intmax_t pid = env->extract_integer (env, args[0]);

  size_t wsLength = 0;
  wchar_t* ws = get_cwd_by_pid((DWORD) pid, &wsLength);
  if (ws == NULL) {
    return env->intern (env, "nil");
  }

  size_t length = 0;
  char* s = narrow(ws, wsLength, &length);
  // TODO do we need to release s ourselves?
  emacs_value utf8Path = env->make_string (env, s, length);

  free(ws);

  return utf8Path;
}

/* Bind NAME to FUN.  */
static void bind_function (emacs_env *env, const char *name, emacs_value Sfun) {
  /* Set the function cell of the symbol named NAME to SFUN using
     the 'fset' function.  */

  /* Convert the strings to symbols by interning them */
  emacs_value Qfset = env->intern (env, "fset");
  emacs_value Qsym = env->intern (env, name);

  /* Prepare the arguments array */
  emacs_value args[] = { Qsym, Sfun };

  /* Make the call (2 == nb of arguments) */
  env->funcall (env, Qfset, 2, args);
}

/* Provide FEATURE to Emacs.  */
static void provide (emacs_env *env, const char *feature) {
  /* call 'provide' with FEATURE converted to a symbol */

  emacs_value Qfeat = env->intern (env, feature);
  emacs_value Qprovide = env->intern (env, "provide");
  emacs_value args[] = { Qfeat };

  env->funcall (env, Qprovide, 1, args);
}

int
emacs_module_init (struct emacs_runtime *ert)
{
  emacs_env *env = ert->get_environment (ert);

  /* create a lambda (returns an emacs_value) */
  emacs_value fun = env->make_function (env,
              1,            /* min. number of arguments */
              1,            /* max. number of arguments */
              Fget_cwd_by_pid,  /* actual function pointer */
              "Get the current working directory of the process with pid PID.",        /* docstring */
              NULL          /* user pointer of your choice (data param in Fget_cwd_by_pid) */
  );

  bind_function (env, "get-cwd-by-pid", fun);
  provide (env, "libemacs-get-cwd-by-pid");

  /* loaded successfully */
  return 0;
}
