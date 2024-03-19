package uk.minersonline.Minecart.resource;

import java.nio.file.Path;
import java.util.Objects;

public class ResourceIdentifier {
	private final String namespace;
	private final String path;
	public final static String DEFAULT_NAMESPACE = "valkyrie";

	public ResourceIdentifier(String namespace, String path) {
		this.namespace = namespace;
		this.path = path;
		isValid();
	}

	private ResourceIdentifier(String[] id) {
		this(id[0], id[1]);
	}

	public ResourceIdentifier(String id) {
		this(split(id));
	}

	protected static String[] split(String id) {
		String[] strings = new String[]{DEFAULT_NAMESPACE, id};
		int i = id.indexOf(':');
		if (i >= 0) {
			strings[1] = id.substring(i + 1);
			if (i >= 1) {
				strings[0] = id.substring(0, i);
			}
		}

		return strings;
	}

	private void isValid() {
		if (isValidNamespace(this.namespace) && isValidPath(this.path)) {
			return;
		}
		if (!isValidNamespace(this.namespace)) {
			throw new RuntimeException(new ResourceIdentifierException(ResourceIdentifierException.Error.NAMESPACE_ERROR));
		}
		if (!isValidPath(this.path)) {
			throw new RuntimeException(new ResourceIdentifierException(ResourceIdentifierException.Error.PATH_ERROR));
		}
	}

	public Path toPath() {
		return Path.of(namespace, path);
	}

	private static boolean isValidNamespace(String namespace) {
		return namespace.matches("^[a-z0-9._-]+$");
	}

	private static boolean isValidPath(String path) {
		return path.matches("^[a-z0-9/._-]+$");
	}

	@Override
	public String toString() {
		return namespace + ":" + path;
	}

	public String getNamespace() {
		return namespace;
	}

	public String getPath() {
		return path;
	}

	@Override
	public boolean equals(Object o) {
		if (this == o) return true;
		if (o == null || getClass() != o.getClass()) return false;
		ResourceIdentifier that = (ResourceIdentifier) o;
		return Objects.equals(namespace, that.namespace) && Objects.equals(path, that.path);
	}

	@Override
	public int hashCode() {
		return Objects.hash(namespace, path);
	}
}
