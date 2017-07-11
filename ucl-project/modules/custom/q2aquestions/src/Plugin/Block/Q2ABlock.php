<?php

namespace Drupal\q2aquestions\Plugin\Block;

use Drupal\Core\Block\BlockBase;
use Drupal\Core\Block\BlockPluginInterface;
use Drupal\Core\Form\FormStateInterface;

/**
 * Provides a 'Hello' Block.
 *
 * @Block(
 *   id = "q2a_questions",
 *   admin_label = @Translation("q2a questions"),
 *   category = @Translation("q2a questions"),
 * )
 */
class Q2ABlock extends BlockBase implements BlockPluginInterface {
  /**
   * {@inheritdoc}
   */
  public function build() {
    global $build;


  //$items = $this->getQuestions();

    $items = array();

    $query = \Drupal::database()->select('qa_posts', 'posts');
    
    $query->leftJoin('qa_categories', 'cat1', 'posts.catidpath1 = cat1.categoryid');
    $query->join('users_field_data', 'users', 'posts.userid = users.uid');
    $query->leftJoin('qa_categories', 'cat2', 'posts.catidpath2 = cat2.categoryid');
    $query->leftJoin('qa_categories', 'cat3', 'posts.catidpath3 = cat2.categoryid');

    
    $query->addField('posts', 'title');
    $query->addField('posts', 'content');
    $query->addField('posts', 'tags');
    $query->addField('posts', 'userid');
    $query->addField('posts', 'postid');
    $query->addField('posts', 'categoryid');
    $query->addField('posts', 'created');
    $query->addField('posts', 'updated');
    $query->addField('cat1', 'title', 'cat1title');
    $query->addField('cat2', 'title', 'cat2title');
    $query->addField('cat3', 'title', 'cat3title');
    $query->addField('cat1', 'tags', 'cat1tags');
    $query->addField('cat2', 'tags', 'cat2tags');
    $query->addField('cat3', 'tags', 'cat3tags');
    $query->addField('users', 'name', 'username');

    $result = $query->execute()->fetchAllAssoc('postid');

    usort($result, function ($a, $b) {
      return new \DateTime($a->updated?$a->updated:$a->created) < new \DateTime($b->updated?$b->updated:$b->created);
    });

    foreach ($result as $value) {
      $item = array('value' => $value->title,
                    'content' => $value->content,
                    'postid' => $value->postid,
                    'tags' => $value->tags,
                    'created' => $value->created,
                    'updated' => $value->updated,
                    'username' => $value->username,
                    'cat1title' => $value->cat1title,
                    'cat2title' => $value->cat2title,
                    'cat3title' => $value->cat3title,
                    'tags' => $value->tags,
                    'tags' => $value->tags,
                    'tags' => $value->tags);
      array_push($items, $item);
    }

    return array(
      '#theme' => 'q2aquestions-question-test',
      '#items' => $items,
      '#cache' => ['max-age' => 0,],
    );
  }

  public function getQuestions() {

  }



  

}