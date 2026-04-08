// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "CreaturePhysics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/BodyInstance.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UCreaturePhysics::UCreaturePhysics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values for medium-sized creature (human/velociraptor scale)
    CreatureSize = ECreatureSize::Medium;
    CurrentPhysicsState = ECreaturePhysicsState::Alive;
    CreatureMass = 100.0f;
    CreatureHeight = 180.0f;
    bIsGroundStable = true;
    MaxStableSlope = 45.0f;
    IKAdjustmentStrength = 0.8f;
    RagdollTransitionTime = 1.0f;
    CollisionDamageThreshold = 500.0f;
    GroundTraceDistance = 200.0f;
    
    // Initialize physics scaling data for different creature sizes
    PhysicsScalingData.Add(ECreatureSize::Small, {0.5f, 0.3f, 0.2f, 30.0f});
    PhysicsScalingData.Add(ECreatureSize::Medium, {1.0f, 1.0f, 1.0f, 90.0f});
    PhysicsScalingData.Add(ECreatureSize::Large, {5.0f, 3.0f, 3.0f, 250.0f});
    PhysicsScalingData.Add(ECreatureSize::Massive, {20.0f, 8.0f, 8.0f, 500.0f});
}

void UCreaturePhysics::BeginPlay()
{
    Super::BeginPlay();
    
    // Get reference to creature's skeletal mesh
    CreatureMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    if (!CreatureMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("CreaturePhysics: No skeletal mesh component found on %s"), 
               *GetOwner()->GetName());
        return;
    }
    
    // Initialize physics settings
    InitializePhysicsForSize();
    ApplyPhysicsConstraints();
    
    UE_LOG(LogTemp, Log, TEXT("CreaturePhysics: Initialized for %s with mass %f kg"), 
           *GetOwner()->GetName(), CreatureMass);
}

void UCreaturePhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Only update ground stability for living creatures
    if (CurrentPhysicsState == ECreaturePhysicsState::Alive)
    {
        UpdateGroundStability();
    }
}

void UCreaturePhysics::ConfigureCreaturePhysics(ECreatureSize CreatureType, float Mass, float Height)
{
    CreatureSize = CreatureType;
    CreatureMass = FMath::Clamp(Mass, 1.0f, 50000.0f);
    CreatureHeight = FMath::Clamp(Height, 10.0f, 2000.0f);
    
    // Update ground trace distance based on height
    GroundTraceDistance = CreatureHeight * 1.2f;
    
    // Reinitialize physics with new parameters
    if (CreatureMesh)
    {
        InitializePhysicsForSize();
        ApplyPhysicsConstraints();
    }
    
    UE_LOG(LogTemp, Log, TEXT("CreaturePhysics: Reconfigured creature physics - Size: %d, Mass: %f kg, Height: %f cm"), 
           static_cast<int32>(CreatureType), Mass, Height);
}

void UCreaturePhysics::ApplyGroundAdaptation(FVector GroundNormal, const TArray<FVector>& FootPositions)
{
    if (!CreatureMesh || CurrentPhysicsState != ECreaturePhysicsState::Alive)
    {
        return;
    }
    
    // Calculate ground slope angle
    float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(GroundNormal, FVector::UpVector)));
    
    // Check if ground is stable
    bIsGroundStable = SlopeAngle <= MaxStableSlope;
    
    // Apply IK adjustments for foot placement
    if (bIsGroundStable && FootPositions.Num() > 0)
    {
        // In a full implementation, this would communicate with the animation system
        // to adjust foot IK targets based on ground surface
        
        for (int32 i = 0; i < FootPositions.Num(); i++)
        {
            // Trace down from each foot position to find ground contact
            FVector TraceStart = FootPositions[i] + FVector::UpVector * 50.0f;
            FVector TraceEnd = FootPositions[i] - FVector::UpVector * 100.0f;
            
            FHitResult HitResult;
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(GetOwner());
            
            bool bHit = GetWorld()->LineTraceSingleByChannel(
                HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams);
            
            if (bHit)
            {
                // Calculate IK adjustment (this would be passed to animation system)
                float IKOffset = (FootPositions[i].Z - HitResult.Location.Z) * IKAdjustmentStrength;
                
                // Debug visualization
                if (CVarShowPhysicsDebug.GetValueOnGameThread())
                {
                    DrawDebugLine(GetWorld(), TraceStart, HitResult.Location, FColor::Green, false, 0.1f);
                    DrawDebugSphere(GetWorld(), HitResult.Location, 5.0f, 8, FColor::Yellow, false, 0.1f);
                }\n            }\n        }\n    }\n    \n    UE_LOG(LogTemp, VeryVerbose, TEXT(\"CreaturePhysics: Ground adaptation - Slope: %f°, Stable: %s\"), \n           SlopeAngle, bIsGroundStable ? TEXT(\"Yes\") : TEXT(\"No\"));\n}\n\nvoid UCreaturePhysics::TransitionToDeathState(FVector DeathImpulse, FVector DeathLocation)\n{\n    if (CurrentPhysicsState == ECreaturePhysicsState::Dead)\n    {\n        return; // Already dead\n    }\n    \n    // Transition to dying state first\n    HandleStateTransition(ECreaturePhysicsState::Dying);\n    \n    if (!CreatureMesh)\n    {\n        UE_LOG(LogTemp, Error, TEXT(\"CreaturePhysics: Cannot transition to death state without skeletal mesh\"));\n        return;\n    }\n    \n    // Start ragdoll transition timer\n    GetWorld()->GetTimerManager().SetTimer(\n        RagdollTransitionTimer,\n        [this, DeathImpulse, DeathLocation]()\n        {\n            // Enable full ragdoll physics\n            CreatureMesh->SetSimulatePhysics(true);\n            CreatureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);\n            CreatureMesh->SetCollisionResponseToAllChannels(ECR_Block);\n            CreatureMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);\n            \n            // Apply death impulse with size-appropriate scaling\n            const FCreaturePhysicsScaling& Scaling = PhysicsScalingData[CreatureSize];\n            FVector ScaledImpulse = DeathImpulse * Scaling.ForceMultiplier;\n            \n            CreatureMesh->AddImpulseAtLocation(ScaledImpulse, DeathLocation);\n            \n            // Transition to final dead state\n            HandleStateTransition(ECreaturePhysicsState::Dead);\n            \n            UE_LOG(LogTemp, Log, TEXT(\"CreaturePhysics: %s transitioned to ragdoll death state with impulse %s\"), \n                   *GetOwner()->GetName(), *ScaledImpulse.ToString());\n        },\n        RagdollTransitionTime,\n        false\n    );\n    \n    UE_LOG(LogTemp, Log, TEXT(\"CreaturePhysics: %s beginning death transition\"), *GetOwner()->GetName());\n}\n\nvoid UCreaturePhysics::HandleCreatureCollision(AActor* OtherCreature, float CollisionImpulse, FVector CollisionLocation)\n{\n    if (!OtherCreature || CurrentPhysicsState == ECreaturePhysicsState::Dead)\n    {\n        return;\n    }\n    \n    // Get size scaling for damage calculation\n    const FCreaturePhysicsScaling& MyScaling = PhysicsScalingData[CreatureSize];\n    \n    // Calculate collision damage based on impulse and size\n    float CollisionDamage = CollisionImpulse / MyScaling.DamageResistance;\n    \n    if (CollisionDamage > CollisionDamageThreshold)\n    {\n        // Significant collision - apply stun effect\n        if (CurrentPhysicsState == ECreaturePhysicsState::Alive)\n        {\n            HandleStateTransition(ECreaturePhysicsState::Stunned);\n            \n            // Auto-recover from stun after a delay\n            GetWorld()->GetTimerManager().SetTimer(\n                FTimerHandle(),\n                [this]()\n                {\n                    if (CurrentPhysicsState == ECreaturePhysicsState::Stunned)\n                    {\n                        HandleStateTransition(ECreaturePhysicsState::Alive);\n                    }\n                },\n                2.0f, // 2 second stun duration\n                false\n            );\n        }\n        \n        // Apply collision response force\n        if (CreatureMesh)\n        {\n            FVector CollisionForce = (GetOwner()->GetActorLocation() - CollisionLocation).GetSafeNormal();\n            CollisionForce *= CollisionImpulse * 0.5f; // Reduce force for realism\n            \n            CreatureMesh->AddImpulseAtLocation(CollisionForce, CollisionLocation);\n        }\n    }\n    \n    UE_LOG(LogTemp, Log, TEXT(\"CreaturePhysics: %s collision with %s - Impulse: %f, Damage: %f\"), \n           *GetOwner()->GetName(), *OtherCreature->GetName(), CollisionImpulse, CollisionDamage);\n}\n\nvoid UCreaturePhysics::InitializePhysicsForSize()\n{\n    if (!CreatureMesh)\n    {\n        return;\n    }\n    \n    const FCreaturePhysicsScaling& Scaling = PhysicsScalingData[CreatureSize];\n    \n    // Set mass based on creature size\n    float ScaledMass = CreatureMass * Scaling.MassMultiplier;\n    \n    // Apply mass to all physics bodies\n    if (UPhysicsAsset* PhysicsAsset = CreatureMesh->GetPhysicsAsset())\n    {\n        for (int32 i = 0; i < PhysicsAsset->SkeletalBodySetups.Num(); i++)\n        {\n            if (UBodySetup* BodySetup = PhysicsAsset->SkeletalBodySetups[i])\n            {\n                // Set mass properties\n                BodySetup->PhysicsType = EPhysicsType::PhysType_Simulated;\n                \n                // Configure collision\n                BodySetup->CollisionTraceFlag = CTF_UseComplexAsSimple;\n                BodySetup->bGenerateNonMirroredCollision = true;\n                BodySetup->bGenerateMirroredCollision = true;\n            }\n        }\n    }\n    \n    // Configure collision capsule size\n    if (UCapsuleComponent* CapsuleComp = GetOwner()->FindComponentByClass<UCapsuleComponent>())\n    {\n        float CapsuleRadius = Scaling.CollisionRadius;\n        float CapsuleHeight = CreatureHeight * 0.5f;\n        \n        CapsuleComp->SetCapsuleSize(CapsuleRadius, CapsuleHeight);\n        CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);\n        CapsuleComp->SetCollisionObjectType(ECC_Pawn);\n    }\n    \n    UE_LOG(LogTemp, Log, TEXT(\"CreaturePhysics: Initialized physics for size %d - Mass multiplier: %f, Collision radius: %f\"), \n           static_cast<int32>(CreatureSize), Scaling.MassMultiplier, Scaling.CollisionRadius);\n}\n\nvoid UCreaturePhysics::UpdateGroundStability()\n{\n    if (!CreatureMesh)\n    {\n        return;\n    }\n    \n    // Trace down to check ground surface\n    FVector TraceStart = GetOwner()->GetActorLocation();\n    FVector TraceEnd = TraceStart - FVector::UpVector * GroundTraceDistance;\n    \n    FHitResult HitResult;\n    FCollisionQueryParams QueryParams;\n    QueryParams.AddIgnoredActor(GetOwner());\n    \n    bool bHit = GetWorld()->LineTraceSingleByChannel(\n        HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams);\n    \n    if (bHit)\n    {\n        // Calculate ground slope\n        float SlopeAngle = FMath::RadiansToDegrees(\n            FMath::Acos(FVector::DotProduct(HitResult.Normal, FVector::UpVector)));\n        \n        bIsGroundStable = SlopeAngle <= MaxStableSlope;\n        \n        // Apply ground adaptation if needed\n        if (!bIsGroundStable && CurrentPhysicsState == ECreaturePhysicsState::Alive)\n        {\n            // Creature is on unstable ground - apply sliding physics\n            FVector SlideDirection = FVector::VectorPlaneProject(FVector::DownVector, HitResult.Normal).GetSafeNormal();\n            FVector SlideForce = SlideDirection * CreatureMass * 100.0f; // Gravity-based sliding\n            \n            if (CreatureMesh->IsSimulatingPhysics())\n            {\n                CreatureMesh->AddForce(SlideForce);\n            }\n        }\n    }\n    else\n    {\n        // No ground detected - creature is falling\n        bIsGroundStable = false;\n    }\n}\n\nvoid UCreaturePhysics::ApplyPhysicsConstraints()\n{\n    if (!CreatureMesh)\n    {\n        return;\n    }\n    \n    const FCreaturePhysicsScaling& Scaling = PhysicsScalingData[CreatureSize];\n    \n    // Configure physics constraints based on creature size\n    CreatureMesh->SetLinearDamping(0.1f * Scaling.MassMultiplier);\n    CreatureMesh->SetAngularDamping(0.1f * Scaling.MassMultiplier);\n    \n    // Set collision responses\n    CreatureMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);\n    CreatureMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);\n    CreatureMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);\n    \n    // Configure mass properties\n    if (CreatureMesh->GetBodyInstance())\n    {\n        CreatureMesh->GetBodyInstance()->SetMassOverride(CreatureMass * Scaling.MassMultiplier, true);\n        CreatureMesh->GetBodyInstance()->bLockXRotation = false;\n        CreatureMesh->GetBodyInstance()->bLockYRotation = false;\n        CreatureMesh->GetBodyInstance()->bLockZRotation = false;\n    }\n    \n    UE_LOG(LogTemp, Log, TEXT(\"CreaturePhysics: Applied physics constraints for %s\"), \n           *GetOwner()->GetName());\n}\n\nvoid UCreaturePhysics::HandleStateTransition(ECreaturePhysicsState NewState)\n{\n    if (CurrentPhysicsState == NewState)\n    {\n        return;\n    }\n    \n    ECreaturePhysicsState PreviousState = CurrentPhysicsState;\n    CurrentPhysicsState = NewState;\n    \n    // Handle state-specific physics changes\n    switch (NewState)\n    {\n        case ECreaturePhysicsState::Alive:\n            if (CreatureMesh)\n            {\n                CreatureMesh->SetSimulatePhysics(false);\n                CreatureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);\n            }\n            break;\n            \n        case ECreaturePhysicsState::Stunned:\n            // Reduce movement capability but keep collision\n            if (CreatureMesh)\n            {\n                CreatureMesh->SetLinearDamping(5.0f); // Increase damping for sluggish movement\n            }\n            break;\n            \n        case ECreaturePhysicsState::Dying:\n            // Prepare for ragdoll transition\n            if (CreatureMesh)\n            {\n                CreatureMesh->SetLinearDamping(0.5f);\n                CreatureMesh->SetAngularDamping(0.5f);\n            }\n            break;\n            \n        case ECreaturePhysicsState::Dead:\n            // Full ragdoll physics enabled\n            if (CreatureMesh)\n            {\n                CreatureMesh->SetSimulatePhysics(true);\n                CreatureMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);\n            }\n            break;\n    }\n    \n    UE_LOG(LogTemp, Log, TEXT(\"CreaturePhysics: %s state transition from %d to %d\"), \n           *GetOwner()->GetName(), static_cast<int32>(PreviousState), static_cast<int32>(NewState));\n}\n\n// Console variable for debug visualization\nstatic TAutoConsoleVariable<bool> CVarShowPhysicsDebug(\n    TEXT(\"tp.ShowPhysicsDebug\"),\n    false,\n    TEXT(\"Show physics debug information for creatures\"),\n    ECVF_Default\n);