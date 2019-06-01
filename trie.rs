use crate::KV;

use std::io;
use std::mem::swap;

extern crate log;
use log::debug;

pub struct Trie {
    root: TrieNode,
    // size in bytes
    size: usize,
}

const TRIE_BITS: usize = 4;
const TRIE_WIDTH: usize = 1 << TRIE_BITS;
const TRIE_MASK: usize = TRIE_WIDTH - 1;
const CAPACITY: usize = 0x400_000;

struct TrieNode {
    size: u32,
    children: [Option<Box<TrieNode>>; TRIE_WIDTH],
    value: Option<Vec<u8>>,
}

impl Trie {
    pub fn new() -> Trie {
        Trie::default()
    }

    fn put_helper(
        p: &mut TrieNode,
        key: &[u8],
        value: Vec<u8>,
        is_highbits: bool,
    ) -> Option<Vec<u8>> {
        if key.is_empty() {
            let ret = if p.value.is_none() {
                p.size += 1;
                None
            } else {
                let mut r = None;
                swap(&mut p.value, &mut r);
                r
            };
            p.value = Some(value);
            return ret;
        }

        // Not the needed key
        let mut c = key[0] as usize;
        if is_highbits {
            c >>= TRIE_BITS;
        } else {
            c &= TRIE_MASK;
        }
        if p.children[c].is_none() {
            p.children[c] = Some(Box::new(TrieNode::new()));
        }
        let child = &mut **p.children[c].as_mut().unwrap();
        let ret = if is_highbits {
            Trie::put_helper(child, &key[1..], value, !is_highbits)
        } else {
            Trie::put_helper(child, key, value, !is_highbits)
        };

        if ret.is_none() {
            p.size += 1;
        }

        ret
    }

    fn delete_helper(p: &mut TrieNode, key: &[u8], is_highbits: bool) -> Option<Vec<u8>> {
        if key.is_empty() {
            let mut ret = None;
            swap(&mut ret, &mut p.value);
            if ret.is_some() {
                p.size -= 1;
            }
            return ret;
        }

        let mut c = key[0] as usize;
        if is_highbits {
            c >>= TRIE_BITS;
        } else {
            c &= TRIE_MASK;
        }

        let child = &mut **p.children[c].as_mut()?;

        let ret = if is_highbits {
            Trie::delete_helper(child, &key[1..], !is_highbits)
        } else {
            Trie::delete_helper(child, &key, !is_highbits)
        };

        if ret.is_some() {
            p.size -= 1;
            if child.size == 0 {
                p.children[c] = None;
            }
        }

        ret
    }

    pub fn is_empty(&self) -> bool {
        self.size == 0
    }

    /* pub fn get_size(&self) -> usize {
        self.size
    } */

    pub fn full(&self) -> bool {
        self.size >= CAPACITY
    }

    pub fn for_each<F>(&self, mut callback: F)
    where
        F: FnMut(&[u8], &[u8]),
    {
        Trie::for_each_helper(&self.root, &mut callback, &mut Vec::new(), false);
    }

    fn for_each_helper<F>(p: &TrieNode, callback: &mut F, key: &mut Vec<u8>, is_highbits: bool)
    where
        F: FnMut(&[u8], &[u8]),
    {
        if p.value.is_some() {
            callback(key, p.value.as_ref().unwrap());
        }
        if !is_highbits {
            key.push(0 as u8);
        }
        for (i, child) in p.children.iter().enumerate() {
            if child.is_none() {
                continue;
            }
            let c = key.last_mut().unwrap();
            if is_highbits {
                *c &= TRIE_MASK; // Clear the modified high bits
                *c |= (i << TRIE_BITS) as u8;
            } else {
                *c = i as u8;
            }
            Trie::for_each_helper(child.as_ref().unwrap(), callback, key, !is_highbits);
        }
        if !is_highbits {
            key.pop();
        }
    }
}

impl Default for Trie {
    fn default() -> Trie {
        Trie {
            root: TrieNode::default(),
            size: 0,
        }
    }
}

impl TrieNode {
    fn new() -> TrieNode {
        TrieNode::default()
    }
}

impl Default for TrieNode {
    fn default() -> TrieNode {
        TrieNode {
            size: 0,
            children: Default::default(),
            value: None,
        }
    }
}

impl KV for Trie {
    fn get(&self, key: &[u8]) -> Option<Vec<u8>> {
        debug!("Trie::getting");
        let mut p: &TrieNode = &self.root;
        for &c in key {
            p = &**p.children[(c as usize) & TRIE_MASK].as_ref()?;
            p = &**p.children[(c as usize) >> TRIE_BITS].as_ref()?;
        }

        debug!("pvalue {}", p.value.is_some());

        p.value.clone()
    }

    fn put(&mut self, key: &[u8], value: Vec<u8>) -> io::Result<()> {
        // TODO: use returned result
        self.size += key.len() + value.len();
        let res = Trie::put_helper(&mut self.root, key, value, false);
        if res.is_some() {
            self.size -= res.unwrap().len();
        }
        Ok(())
    }
    fn delete(&mut self, key: &[u8]) -> io::Result<()> {
        // TODO: use returned result
        let ret = Trie::delete_helper(&mut self.root, key, false);
        if ret.is_some() {
            // TODO: check the semantic
            self.size -= key.len() + ret.as_ref().unwrap().len()
        }
        Ok(())
    }
}
