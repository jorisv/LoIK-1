//
// Copyright (c) 2024 INRIA
//

#pragma once

#include "loik/config.hpp"
#include "loik/macros.hpp"

#include <pinocchio/math/tensor.hpp>
#include <pinocchio/spatial/fwd.hpp>

#include <pinocchio/container/aligned-vector.hpp>
#include <pinocchio/multibody/fwd.hpp>
#include <pinocchio/multibody/joint/joint-generic.hpp>
#include <pinocchio/spatial/force.hpp>
#include <pinocchio/spatial/inertia.hpp>
#include <pinocchio/spatial/motion.hpp>
#include <pinocchio/spatial/se3.hpp>

#include <pinocchio/serialization/serializable.hpp>

#include <Eigen/StdVector>
#include <Eigen/src/Core/util/Constants.h>

#include <boost/foreach.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/range/combine.hpp>

#include <cstddef>
#include <set>

namespace loik
{

  template<
    typename _Scalar,
    int _Options = 0,
    template<typename S, int O> class JointCollectionTpl = pinocchio::JointCollectionDefaultTpl>
  struct IkIdDataTypeOptimizedTpl;

}

namespace pinocchio
{

  template<typename _Scalar, int _Options, template<typename, int> class JointCollectionTpl>
  struct traits<loik::IkIdDataTypeOptimizedTpl<_Scalar, _Options, JointCollectionTpl>>
  {
    typedef _Scalar Scalar;
  };

} // namespace pinocchio

namespace loik
{

  // // fwd declaration of pinocchio::ik_id::traits
  // // TODO: move this to pinocchio/fwd.hpp
  // template<typename T> struct traits {};

  // // NumericalBase for namespace ik_id
  // template<class Derived>
  // struct NumericalBase
  // {
  // typedef typename traits<Derived>::Scalar Scalar;
  // };

  // IkIdDataTpl definition
  template<typename _Scalar, int _Options, template<typename S, int> class JointCollectionTpl>
  struct IkIdDataTypeOptimizedTpl
  : pinocchio::serialization::Serializable<
      IkIdDataTypeOptimizedTpl<_Scalar, _Options, JointCollectionTpl>>
  , pinocchio::NumericalBase<IkIdDataTypeOptimizedTpl<_Scalar, _Options, JointCollectionTpl>>
  {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    typedef _Scalar Scalar;
    enum
    {
      Options = _Options
    };

    typedef pinocchio::ModelTpl<Scalar, Options, JointCollectionTpl> Model;

    typedef pinocchio::SE3Tpl<Scalar, Options> SE3;
    typedef pinocchio::MotionTpl<Scalar, Options> Motion;
    typedef pinocchio::ForceTpl<Scalar, Options> Force;
    typedef pinocchio::InertiaTpl<Scalar, Options> Inertia;
    typedef pinocchio::FrameTpl<Scalar, Options> Frame;

    typedef pinocchio::Index Index;
    typedef pinocchio::JointIndex JointIndex;
    typedef pinocchio::FrameIndex FrameIndex;
    typedef std::vector<Index> IndexVector;

    typedef pinocchio::JointModelTpl<Scalar, Options, JointCollectionTpl> JointModel;
    typedef pinocchio::JointDataTpl<Scalar, Options, JointCollectionTpl> JointData;

    typedef PINOCCHIO_ALIGNED_STD_VECTOR(JointModel) JointModelVector;
    typedef PINOCCHIO_ALIGNED_STD_VECTOR(JointData) JointDataVector;

    typedef Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic, Options> DMat;
    typedef Eigen::Matrix<Scalar, Eigen::Dynamic, 1, Options> DVec;
    typedef Eigen::Matrix<Scalar, 3, 1, Options> Vec3;
    typedef Eigen::Matrix<Scalar, 6, 1, Options> Vec6;
    typedef Eigen::Matrix<Scalar, 6, 6> Mat6x6;

    /// \brief Dense vectorized version of a joint configuration vector, q.
    typedef DVec ConfigVectorType;

    /// \brief Dense vectorized version of a joint tangent vector (e.g. velocity,
    /// acceleration, etc), q_dot.
    ///        It also handles the notion of co-tangent vector (e.g. torque, etc).
    typedef DVec TangentVectorType;

    /// \brief Vector of pinocchio::JointData associated to the
    /// pinocchio::JointModel stored in model, encapsulated in JointDataAccessor.
    JointDataVector joints;

    /// \brief Vector of absolute joint placements (wrt the world).
    PINOCCHIO_ALIGNED_STD_VECTOR(SE3) oMi;

    /// \brief Vector of relative joint placements (wrt the body parent).
    PINOCCHIO_ALIGNED_STD_VECTOR(SE3) liMi;

    /// \brief generalized velocity
    DVec nu;

    /// \brief generalized velocity from the previous solver iteration
    DVec nu_prev;

    /// \brief link spatial velocity expressed in link's local frame
    PINOCCHIO_ALIGNED_STD_VECTOR(Motion) vis;

    /// \brief link spatial velocity expressed at the origin of the world. TODO:
    /// not used
    PINOCCHIO_ALIGNED_STD_VECTOR(Motion) ovis;

    /// \brief link spatial velocity expressed in link's local frame from the
    /// previous solver iteration
    PINOCCHIO_ALIGNED_STD_VECTOR(Motion) vis_prev;

    /// \brief "state-feedback" matrix term in the recursion hyputhesis for f_i
    PINOCCHIO_ALIGNED_STD_VECTOR(Mat6x6) His;

    /// \brief initialized to 'His' but acts as a container for tmp quantities
    /// that gets overwritten during LOIK bwdpass, after bwdpass, it becomes Pi *
    /// Hi, analogous to 'data.Yaba'
    PINOCCHIO_ALIGNED_STD_VECTOR(Mat6x6) His_aba;

    /// \brief bias vector terms in the recursion hypothesis for f_i
    PINOCCHIO_ALIGNED_STD_VECTOR(Force) pis;

    /// \brief bias vector terms in the recursion hypothesis for f_i
    PINOCCHIO_ALIGNED_STD_VECTOR(Force) pis_aba;

    /// \brief quadratic penalty term associated with nu_i introduced by
    /// inequality constraints on nu_i through ADMM, equivalent to armature
    DVec R;

    /// \brief affine terms associated with nu_i introduced by inequality
    /// constraints on nu_i through ADMM, type joint space "force"
    ///        analogous to 'data.u'
    DVec r;

    /// \brief dual variable for the kinematics/dynamics constraint, i.e.
    /// constraint "forces"
    PINOCCHIO_ALIGNED_STD_VECTOR(Force) fis;

    // ADMM formulation related quantities

    /// TODO: decide on the type for yis
    /// \brief dual variables associated with motion constraints
    PINOCCHIO_ALIGNED_STD_VECTOR(Vec6) yis;

    /// TODO: same type as yis
    /// \brief previous dual variables associated with motion constraints
    PINOCCHIO_ALIGNED_STD_VECTOR(Vec6) yis_prev;

    /// \brief dual vairables associated with inequality slack induced equality
    /// constraints
    DVec w;

    /// \brief previous dual vairables associated with inequality slack induced
    /// equality constraints
    DVec w_prev;

    /// \brief slack variables for inequality constraints
    DVec z;

    /// \brief previous slack variables for inequality constraints
    DVec z_prev;

    ///
    /// \brief untility member, all joint index collected into a std::vector,
    /// including the universe joint
    ///        should be moved to Model
    ///
    IndexVector joint_full_range;

    ///
    /// \brief untility member, all joint index collected into a std::vector,
    /// excluding the universe joint
    ///        should be moved to Model
    ///
    IndexVector joint_range;

    ///
    /// \brief dimension of the task equality constraints (DEPRACATED)
    ///
    int eq_c_dim;

    ///
    /// \brief Ai.transpose * yi  ; updated in the solver after dual variables
    /// (yis) udpate (i.e. in DualUpdate).
    ///        this quantity is used store intermediate results
    ///        for dual residual computation and primal infeasibility checking
    ///
    PINOCCHIO_ALIGNED_STD_VECTOR(Vec6) Aty;

    ///
    /// \brief record of Aty from previous iteration, needed for primal
    /// infeasibility checking
    ///
    PINOCCHIO_ALIGNED_STD_VECTOR(Vec6) Aty_prev;

    ///
    /// \brief \Delta f_i := - f_i + \sum_{j \in \gamma(i)} f_j . Updated in
    /// BwdPass2
    ///        this quantity is used store intermediate results
    ///        for dual residual computation and primal infeasibility checking
    ///
    PINOCCHIO_ALIGNED_STD_VECTOR(Force) fis_diff;

    ///
    /// \brief record of fis_diff from previous iteration, needed for primal
    /// infeasibility checking
    ///
    PINOCCHIO_ALIGNED_STD_VECTOR(Force) fis_diff_prev;

    ///
    /// \brief H_ref * vi, , this quantity is used store intermediate results
    ///        for dual residual computation and dual infeasibility checking.
    ///        Updated in FwdPass2
    ///
    PINOCCHIO_ALIGNED_STD_VECTOR(Vec6) Href_v;

    ///
    /// \brief Ai * vi - bi, this quantity is used to store intermediate results
    ///        for primal residual computation. Updated in DualUpdate
    ///
    PINOCCHIO_ALIGNED_STD_VECTOR(Vec6) Av_minus_b;

    ///
    /// \brief Si.transpose() * fi + wi, this quantity is used store intermediate
    /// results
    ///        for dual residual computation and primal infeasibility checking,
    ///        updated in BwdPass2
    ///
    DVec Stf_plus_w;

    ///
    /// \brief record of Stf_plus_w from previous iteration
    ///
    DVec Stf_plus_w_prev;

    ///
    /// \brief bi.transpose() * yi, this quantity is used store intermediate
    /// results
    ///        for primal infeasibility checking. Updated in DualUpdate
    ///
    PINOCCHIO_ALIGNED_STD_VECTOR(Scalar) bty;

    ///
    /// \brief record of bty from previous iteration, used for primal
    /// infeasibility checking
    ///
    PINOCCHIO_ALIGNED_STD_VECTOR(Scalar) bty_prev;

    ///
    /// \brief default constructor
    ///
    IkIdDataTypeOptimizedTpl() = default;

    ///
    /// \brief construct from pinocchio::ModelTpl
    ///
    /// \param[in] model The model structure of the rigid body system.
    ///
    explicit IkIdDataTypeOptimizedTpl(const Model & model);

    ///
    /// \brief construct from pinocchio::ModelTpl and a task equality constraint
    /// dimension
    ///
    /// \param[in] model The model structure of the rigid body system.
    /// \param[in] eq_c_dim Task Equality constraint dimension.
    ///
    explicit IkIdDataTypeOptimizedTpl(const Model & model, const int eq_c_dim);

    ///
    /// \brief reset all quantities
    ///
    void Reset(const bool warm_start);

    ///
    /// \brief reset all recursion related quantities
    ///
    void ResetRecursion();

    ///
    /// \brief update _prev primal dual variables with that of current values
    ///
    void UpdatePrev();

  }; // struct IkIdDataTpl

  ///
  /// \brief utility function for checking IkIdData against the Model it
  /// instantiated from
  ///
  template<typename Scalar, int Options, template<typename, int> class JointCollectionTpl>
  inline bool checkIkIdData(
    const pinocchio::ModelTpl<Scalar, Options, JointCollectionTpl> & model,
    const IkIdDataTypeOptimizedTpl<Scalar, Options, JointCollectionTpl> & ik_id_data)
  {
    typedef pinocchio::ModelTpl<Scalar, Options, JointCollectionTpl> Model;
    typedef IkIdDataTypeOptimizedTpl<Scalar, Options, JointCollectionTpl> IkIdData;
    typedef typename Model::JointModel JointModel;

    std::cout << (static_cast<int>(ik_id_data.joints.size()) == model.njoints) << std::endl;

#define CHECK_DATA(a)                                                                              \
  if (!(a))                                                                                        \
    return false;

    // check sizes of each data members
    CHECK_DATA(static_cast<int>(ik_id_data.joints.size()) == model.njoints);
    CHECK_DATA(static_cast<int>(ik_id_data.oMi.size()) == model.njoints);
    CHECK_DATA(static_cast<int>(ik_id_data.liMi.size()) == model.njoints);
    CHECK_DATA(static_cast<int>(ik_id_data.nu.size()) == model.nv);
    CHECK_DATA(static_cast<int>(ik_id_data.nu_prev.size()) == model.nv);
    CHECK_DATA(static_cast<int>(ik_id_data.vis.size()) == model.njoints);
    CHECK_DATA(static_cast<int>(ik_id_data.ovis.size()) == model.njoints);
    CHECK_DATA(static_cast<int>(ik_id_data.vis_prev.size()) == model.njoints);
    CHECK_DATA(static_cast<int>(ik_id_data.His_aba.size()) == model.njoints);
    CHECK_DATA(static_cast<int>(ik_id_data.His.size()) == model.njoints);
    CHECK_DATA(static_cast<int>(ik_id_data.pis.size()) == model.njoints);
    CHECK_DATA(static_cast<int>(ik_id_data.pis_aba.size()) == model.njoints);
    CHECK_DATA(static_cast<int>(ik_id_data.R.size()) == model.nv);
    CHECK_DATA(static_cast<int>(ik_id_data.r.size()) == model.nv);
    CHECK_DATA(static_cast<int>(ik_id_data.fis.size()) == model.njoints);
    CHECK_DATA(static_cast<int>(ik_id_data.yis.size()) == model.njoints);
    CHECK_DATA(static_cast<int>(ik_id_data.yis_prev.size()) == model.njoints);
    CHECK_DATA(static_cast<int>(ik_id_data.w.size()) == model.nv);
    CHECK_DATA(static_cast<int>(ik_id_data.w_prev.size()) == model.nv);
    CHECK_DATA(static_cast<int>(ik_id_data.z.size()) == model.nv);
    CHECK_DATA(static_cast<int>(ik_id_data.z_prev.size()) == model.nv);
    CHECK_DATA(static_cast<int>(ik_id_data.joint_full_range.size()) == model.njoints);
    CHECK_DATA(static_cast<int>(ik_id_data.joint_range.size()) == model.njoints - 1);
    CHECK_DATA(static_cast<int>(ik_id_data.Aty.size()) == model.njoints);
    CHECK_DATA(static_cast<int>(ik_id_data.Aty_prev.size()) == model.njoints);
    CHECK_DATA(static_cast<int>(ik_id_data.fis_diff.size()) == model.njoints);
    CHECK_DATA(static_cast<int>(ik_id_data.fis_diff_prev.size()) == model.njoints);
    CHECK_DATA(static_cast<int>(ik_id_data.Href_v.size()) == model.njoints);
    CHECK_DATA(static_cast<int>(ik_id_data.Av_minus_b.size()) == model.njoints);
    CHECK_DATA(static_cast<int>(ik_id_data.Stf_plus_w.size()) == model.nv);
    CHECK_DATA(static_cast<int>(ik_id_data.Stf_plus_w_prev.size()) == model.nv);
    CHECK_DATA(static_cast<int>(ik_id_data.bty.size()) == model.njoints);
    CHECK_DATA(static_cast<int>(ik_id_data.bty_prev.size()) == model.njoints);

    for (pinocchio::JointIndex joint_id = 1;
         joint_id < static_cast<pinocchio::JointIndex>(model.njoints); ++joint_id)
    {
      const JointModel & jmodel = model.joints[joint_id];

      CHECK_DATA(model.nqs[joint_id] == jmodel.nq());
      CHECK_DATA(model.idx_qs[joint_id] == jmodel.idx_q());
      CHECK_DATA(model.nvs[joint_id] == jmodel.nv());
      CHECK_DATA(model.idx_vs[joint_id] == jmodel.idx_v());
    }

    // check ik_id_data utility quantities
    typename IkIdData::IndexVector joint_full_range_test;
    joint_full_range_test.reserve(static_cast<std::size_t>(model.njoints));

    for (typename IkIdData::Index idx = 0; static_cast<int>(idx) < model.njoints; idx++)
    {
      joint_full_range_test.push_back(idx);
    }

    typename IkIdData::IndexVector joint_range_test = joint_full_range_test;
    joint_range_test.erase(joint_range_test.begin());

    CHECK_DATA(ik_id_data.joint_full_range == joint_full_range_test);
    CHECK_DATA(ik_id_data.joint_range == joint_range_test);

#undef CHECK_DATA
    return true;
  }

  ///
  /// \brief utility function for checking IkIdData against pinocchio::Data
  ///
  template<typename Scalar, int Options, template<typename, int> class JointCollectionTpl>
  inline bool checkIkIdData(
    const pinocchio::ModelTpl<Scalar, Options, JointCollectionTpl> & model,
    const IkIdDataTypeOptimizedTpl<Scalar, Options, JointCollectionTpl> & ik_id_data,
    const pinocchio::DataTpl<Scalar, Options, JointCollectionTpl> & data)
  {

    bool size_dim_check = checkIkIdData(model, ik_id_data);

    if (size_dim_check)
    {
#define CHECK_DATA(a)                                                                              \
  if (!(a))                                                                                        \
    return false;

      // check joint data created btw "ik_id_data" and "data" are the same
      for (std::size_t j = 1; j < static_cast<std::size_t>(ik_id_data.joints.size()); ++j)
      {
        CHECK_DATA(ik_id_data.joints.at(j) == data.joints.at(j));
      }

#undef CHECK_DATA
      return true;
    }
    else
    {
      return false;
    }

  } // checkIkIdData

} // namespace loik

#include "loik/loik-loid-data-optimized.hxx"

#if LOIK_ENABLE_TEMPLATE_INSTANTIATION
  #include "loik/loik-loid-data.txx"
#endif // LOIK_ENABLE_TEMPLATE_INSTANTIATION
