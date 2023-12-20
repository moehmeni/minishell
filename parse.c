/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmomeni <mmomeni@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/09 19:11:29 by mmomeni           #+#    #+#             */
/*   Updated: 2023/12/20 16:05:25 by mmomeni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	parse_redirect(char **v, int i)
{
	if (ft_strlen(v[i]) == 1 && (v[i][0] == '<' || v[i][0] == '>'))
	{
		if (v[i][0] == '<')
		{
			if (open(v[i + 1], O_RDONLY) < 0)
				return (terminate(v[i + 1], NULL), 1);
			dup2(open(v[i + 1], O_RDONLY), 0);
		}
		else if (v[i][0] == '>')
			dup2(open(v[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644), 1);
		return (2);
	}
	else if (ft_strlen(v[i]) == 2 && !ft_strncmp(v[i], ">>", 2))
	{
		dup2(open(v[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644), 1);
		return (2);
	}
	return (0);
}

void	parse_env_vars(char **v, char **env)
{
	int		i;
	char	*tmp;
	char	*loc;
	char	*end;
	char	*key;

	i = 0;
	while (v[i])
	{
		loc = v[i];
		while (ft_strchr(loc, '$'))
		{
			loc = ft_strchr(loc, '$');
			end = loc + 1;
			while (ft_isalnum(*end) || *end == '_')
				end++;
			key = ft_substr(loc, 1, end - loc - 1);
			tmp = get_env(env, key);
			v[i] = ft_strrepl(v[i], ft_strjoin("$", key), tmp);
			free(key);
			loc = end;
		}
		i++;
	}
}

t_quote_parsed	parse_quotes(const char *s)
{
	t_quote_parsed	p;
	int				ij[2];

	ij[0] = -1;
	ij[1] = 0;
	p.str = ft_calloc(strlen(s) + 1, sizeof(char));
	p.end = 0;
	while (s[++ij[0]])
	{
		if (s[ij[0]] == '\'' || s[ij[0]] == '"')
		{
			if (p.end == 0)
				p.end = s[ij[0]];
			else if (p.end == s[ij[0]])
				p.end = 0;
			else
				p.str[ij[1]++] = s[ij[0]];
		}
		else
			p.str[ij[1]++] = s[ij[0]];
	}
	return (p);
}
static int	parse_heredoc(char **v, int i)
{
	char	*hd_str;
	char	*end;

	hd_str = NULL;
	end = NULL;
	// Single quotes
	if (parse_quotes(v[i]).end)
		end = (char[2]){parse_quotes(v[i]).end, 0};
	// -----------------------------------------------
	// Here-docs (it is different from the single quotes.
	else if ((ft_strlen(v[i]) == 2 && !ft_strncmp(v[i], "<<", 2)))
		end = v[i + 1];
	else if (ft_strnstr(v[i], "<<", ft_strlen(v[i])))
		end = ft_strchr(v[i], '<') + 2;
	// -----------------------------------------------
	if (end)
		hd_str = here_doc(end);
	if (hd_str)
	{
		if (!ft_strcmp(hd_str, "ERR"))
			return (2);
		print_vec(v);
		v[i] = ft_strrepl(v[i], "<", "");
		v[i] = ft_strrepl(v[i], end, hd_str);
		free(hd_str);
	}
	return (0);
}

char	*parse(char *s, char **env)
{
	int		in[2];
	char	*tmp;
	char	*tmp2;
	char	*tmp3;
	char	**v;
	int		hd;
	int		rd;

	in[0] = 0;
	v = ft_split(s, ' ');
	parse_env_vars(v, env);
	in[1] = ft_veclen(v);
	while (v[in[0]++])
	{
		hd = parse_heredoc(v, in[0] - 1);
		rd = parse_redirect(v, in[0] - 1);
		dprintf(2, "hd: %d, rd: %d\n", hd, rd);
		in[1] -= hd + rd;
	}
	tmp = ft_vecnjoin(v, " ", in[1]);
	tmp2 = ft_strtrim(tmp, " ");
	tmp3 = ft_strrmchr(tmp2, "\\;\"'");
	free(tmp);
	free(tmp2);
	ft_vecfree(v);
	return (tmp3);
}
