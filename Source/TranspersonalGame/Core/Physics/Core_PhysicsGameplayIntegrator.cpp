#include "Core_PhysicsGameplayIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ACore_PhysicsGameplayIntegrator::ACore_PhysicsGameplayIntegrator()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20Hz for gameplay responsiveness
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Initialize system state
    bIsSystemActive = true;
    bEnableCharacterPhysics = true;
    bEnableDinosaurPhysics = true;
    bEnableEnvironmentPhysics = true;
    bEnableInteractionPhysics = true;
    bEnableRagdollIntegration = true;
    bEnableDestructionIntegration = true;
    
    // Gameplay physics parameters
    CharacterPhysicsMultiplier = 1.0f;
    DinosaurPhysicsMultiplier = 1.2f;
    EnvironmentPhysicsMultiplier = 0.8f;
    InteractionForceMultiplier = 1.5f;
    
    // Integration thresholds
    MaxGameplayPhysicsActors = 200;
    MaxInteractionForces = 100;
    GameplayUpdateInterval = 0.1f;
    
    // Performance tracking
    CurrentGameplayActors = 0;
    CurrentInteractionForces = 0;
    GameplayPerformanceScore = 100.0f;
    
    // Initialize arrays
    RegisteredCharacters.Empty();
    RegisteredDinosaurs.Empty();
    RegisteredInteractables.Empty();
    ActiveRagdolls.Empty();
    
    LastGameplayUpdate = 0.0f;
}

void ACore_PhysicsGameplayIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsGameplayIntegrator: System initialized"));
    
    // Register with existing gameplay systems
    RegisterGameplayActors();
    
    // Start gameplay physics monitoring
    GetWorldTimerManager().SetTimer(
        GameplayTimerHandle,
        this,
        &ACore_PhysicsGameplayIntegrator::UpdateGameplayPhysics,
        GameplayUpdateInterval,
        true
    );
}

void ACore_PhysicsGameplayIntegrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bIsSystemActive) return;
    
    // Update character physics integration
    if (bEnableCharacterPhysics)
    {
        UpdateCharacterPhysics(DeltaTime);
    }
    
    // Update dinosaur physics integration
    if (bEnableDinosaurPhysics)
    {
        UpdateDinosaurPhysics(DeltaTime);
    }
    
    // Update interaction physics
    if (bEnableInteractionPhysics)
    {
        UpdateInteractionPhysics(DeltaTime);
    }
    
    // Update ragdoll integration
    if (bEnableRagdollIntegration)
    {
        UpdateRagdollIntegration(DeltaTime);
    }
}

void ACore_PhysicsGameplayIntegrator::RegisterGameplayActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find and register characters
    TArray<AActor*> FoundCharacters;
    UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), FoundCharacters);
    
    for (AActor* Actor : FoundCharacters)
    {
        if (ACharacter* Character = Cast<ACharacter>(Actor))
        {
            RegisteredCharacters.Add(Character);
        }
    }
    
    // Find and register dinosaurs (actors with "Dinosaur" in name)
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        FString ActorName = Actor->GetName();
        if (ActorName.Contains(TEXT("Dinosaur")) || ActorName.Contains(TEXT("TRex")) || 
            ActorName.Contains(TEXT("Raptor")) || ActorName.Contains(TEXT("Brachio")))
        {
            RegisteredDinosaurs.Add(Actor);
        }
        
        // Register interactables (actors with physics simulation)
        if (Actor->GetRootComponent() && Actor->GetRootComponent()->IsSimulatingPhysics())
        {
            RegisteredInteractables.Add(Actor);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsGameplayIntegrator: Registered %d characters, %d dinosaurs, %d interactables"), 
           RegisteredCharacters.Num(), RegisteredDinosaurs.Num(), RegisteredInteractables.Num());
}

void ACore_PhysicsGameplayIntegrator::UpdateCharacterPhysics(float DeltaTime)
{
    for (ACharacter* Character : RegisteredCharacters)
    {
        if (!IsValid(Character)) continue;
        
        // Enhanced character physics integration
        UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent();
        if (CapsuleComp)
        {
            // Apply character-specific physics multiplier
            FVector CurrentVelocity = Character->GetVelocity();
            if (CurrentVelocity.Size() > 0.1f)
            {
                // Enhance physics response based on movement
                CapsuleComp->SetPhysicsLinearVelocity(CurrentVelocity * CharacterPhysicsMultiplier);
            }
        }
        
        // Handle character-environment interactions
        HandleCharacterEnvironmentInteraction(Character, DeltaTime);
    }
}

void ACore_PhysicsGameplayIntegrator::UpdateDinosaurPhysics(float DeltaTime)
{
    for (AActor* Dinosaur : RegisteredDinosaurs)
    {
        if (!IsValid(Dinosaur)) continue;
        
        // Enhanced dinosaur physics
        UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Dinosaur->GetRootComponent());
        if (RootPrim && RootPrim->IsSimulatingPhysics())
        {
            // Apply dinosaur-specific physics multiplier
            FVector CurrentVelocity = RootPrim->GetPhysicsLinearVelocity();
            if (CurrentVelocity.Size() > 0.1f)
            {
                RootPrim->SetPhysicsLinearVelocity(CurrentVelocity * DinosaurPhysicsMultiplier);
            }
            
            // Apply ground interaction forces
            HandleDinosaurGroundInteraction(Dinosaur, DeltaTime);
        }
    }
}

void ACore_PhysicsGameplayIntegrator::UpdateInteractionPhysics(float DeltaTime)
{
    CurrentInteractionForces = 0;
    
    // Process character-object interactions
    for (ACharacter* Character : RegisteredCharacters)
    {
        if (!IsValid(Character)) continue;
        
        FVector CharacterLocation = Character->GetActorLocation();
        
        for (AActor* Interactable : RegisteredInteractables)
        {
            if (!IsValid(Interactable)) continue;
            
            float Distance = FVector::Dist(CharacterLocation, Interactable->GetActorLocation());
            if (Distance < 500.0f) // Interaction range
            {
                HandleCharacterObjectInteraction(Character, Interactable, DeltaTime);
                CurrentInteractionForces++;
            }
        }
    }
    
    // Process dinosaur-object interactions
    for (AActor* Dinosaur : RegisteredDinosaurs)
    {
        if (!IsValid(Dinosaur)) continue;
        
        FVector DinosaurLocation = Dinosaur->GetActorLocation();
        
        for (AActor* Interactable : RegisteredInteractables)
        {
            if (!IsValid(Interactable) || Interactable == Dinosaur) continue;
            
            float Distance = FVector::Dist(DinosaurLocation, Interactable->GetActorLocation());
            if (Distance < 800.0f) // Larger interaction range for dinosaurs
            {
                HandleDinosaurObjectInteraction(Dinosaur, Interactable, DeltaTime);
                CurrentInteractionForces++;
            }
        }
    }
}

void ACore_PhysicsGameplayIntegrator::UpdateRagdollIntegration(float DeltaTime)
{
    // Update active ragdolls
    for (int32 i = ActiveRagdolls.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(ActiveRagdolls[i]))
        {
            ActiveRagdolls.RemoveAt(i);
            continue;
        }
        
        // Apply ragdoll-specific physics updates
        HandleRagdollPhysics(ActiveRagdolls[i], DeltaTime);
    }
}

void ACore_PhysicsGameplayIntegrator::HandleCharacterEnvironmentInteraction(ACharacter* Character, float DeltaTime)
{
    if (!Character) return;
    
    // Trace for ground interaction
    FVector StartLocation = Character->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 200);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Character);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        // Apply ground-specific physics effects
        if (HitResult.GetActor())
        {
            FString SurfaceName = HitResult.GetActor()->GetName();
            
            // Different surface responses
            if (SurfaceName.Contains(TEXT("Mud")) || SurfaceName.Contains(TEXT("Swamp")))
            {
                // Reduce movement speed on soft surfaces
                Character->GetCharacterMovement()->MaxWalkSpeed *= 0.7f;
            }
            else if (SurfaceName.Contains(TEXT("Rock")) || SurfaceName.Contains(TEXT("Stone")))
            {
                // Normal movement on hard surfaces
                Character->GetCharacterMovement()->MaxWalkSpeed *= 1.0f;
            }
        }
    }
}

void ACore_PhysicsGameplayIntegrator::HandleDinosaurGroundInteraction(AActor* Dinosaur, float DeltaTime)
{
    if (!Dinosaur) return;
    
    UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Dinosaur->GetRootComponent());
    if (!RootPrim) return;
    
    // Apply weight-based ground deformation simulation
    FVector DinosaurLocation = Dinosaur->GetActorLocation();
    float DinosaurMass = RootPrim->GetMass();
    
    // Heavy dinosaurs create ground impact effects
    if (DinosaurMass > 1000.0f)
    {
        // Simulate ground tremor effect
        TArray<AActor*> NearbyActors;
        UGameplayStatics::GetAllActorsWithinRadius(GetWorld(), DinosaurLocation, 1000.0f, TArray<TEnumAsByte<EObjectTypeQuery>>(), TArray<AActor*>(), NearbyActors);
        
        for (AActor* NearbyActor : NearbyActors)
        {
            if (NearbyActor == Dinosaur || !IsValid(NearbyActor)) continue;
            
            UPrimitiveComponent* NearbyPrim = Cast<UPrimitiveComponent>(NearbyActor->GetRootComponent());
            if (NearbyPrim && NearbyPrim->IsSimulatingPhysics())
            {
                float Distance = FVector::Dist(DinosaurLocation, NearbyActor->GetActorLocation());
                float TremorForce = FMath::Clamp(1000.0f / Distance, 0.0f, 500.0f);
                
                FVector TremorDirection = (NearbyActor->GetActorLocation() - DinosaurLocation).GetSafeNormal();
                NearbyPrim->AddImpulse(TremorDirection * TremorForce);
            }
        }
    }
}

void ACore_PhysicsGameplayIntegrator::HandleCharacterObjectInteraction(ACharacter* Character, AActor* Object, float DeltaTime)
{
    if (!Character || !Object) return;
    
    UPrimitiveComponent* ObjectPrim = Cast<UPrimitiveComponent>(Object->GetRootComponent());
    if (!ObjectPrim || !ObjectPrim->IsSimulatingPhysics()) return;
    
    // Calculate interaction force based on character movement
    FVector CharacterVelocity = Character->GetVelocity();
    if (CharacterVelocity.Size() > 100.0f) // Character is moving fast enough
    {
        FVector InteractionDirection = (Object->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal();
        float InteractionForce = CharacterVelocity.Size() * InteractionForceMultiplier;
        
        ObjectPrim->AddImpulse(InteractionDirection * InteractionForce);
        
        UE_LOG(LogTemp, Log, TEXT("Character-Object Interaction: Force=%.1f"), InteractionForce);
    }
}

void ACore_PhysicsGameplayIntegrator::HandleDinosaurObjectInteraction(AActor* Dinosaur, AActor* Object, float DeltaTime)
{
    if (!Dinosaur || !Object) return;
    
    UPrimitiveComponent* DinosaurPrim = Cast<UPrimitiveComponent>(Dinosaur->GetRootComponent());
    UPrimitiveComponent* ObjectPrim = Cast<UPrimitiveComponent>(Object->GetRootComponent());
    
    if (!DinosaurPrim || !ObjectPrim || !ObjectPrim->IsSimulatingPhysics()) return;
    
    // Dinosaurs can push objects with their mass
    float DinosaurMass = DinosaurPrim->GetMass();
    FVector DinosaurVelocity = DinosaurPrim->GetPhysicsLinearVelocity();
    
    if (DinosaurVelocity.Size() > 50.0f && DinosaurMass > 500.0f)
    {
        FVector PushDirection = (Object->GetActorLocation() - Dinosaur->GetActorLocation()).GetSafeNormal();
        float PushForce = (DinosaurMass * DinosaurVelocity.Size()) * 0.1f;
        
        ObjectPrim->AddImpulse(PushDirection * PushForce);
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur-Object Interaction: Mass=%.1f, Force=%.1f"), DinosaurMass, PushForce);
    }
}

void ACore_PhysicsGameplayIntegrator::HandleRagdollPhysics(AActor* RagdollActor, float DeltaTime)
{
    if (!RagdollActor) return;
    
    USkeletalMeshComponent* SkeletalMesh = RagdollActor->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMesh) return;
    
    // Apply ragdoll-specific physics adjustments
    if (SkeletalMesh->IsSimulatingPhysics())
    {
        // Reduce ragdoll energy over time
        TArray<FName> BoneNames;
        SkeletalMesh->GetBoneNames(BoneNames);
        
        for (const FName& BoneName : BoneNames)
        {
            FVector BoneVelocity = SkeletalMesh->GetPhysicsLinearVelocity(BoneName);
            if (BoneVelocity.Size() > 10.0f)
            {
                // Apply damping to reduce excessive ragdoll movement
                FVector DampedVelocity = BoneVelocity * 0.95f;
                SkeletalMesh->SetPhysicsLinearVelocity(DampedVelocity, false, BoneName);
            }
        }
    }
}

void ACore_PhysicsGameplayIntegrator::UpdateGameplayPhysics()
{
    LastGameplayUpdate = GetWorld()->GetTimeSeconds();
    
    // Update performance metrics
    CurrentGameplayActors = RegisteredCharacters.Num() + RegisteredDinosaurs.Num();
    
    // Calculate performance score
    float ActorLoad = (float)CurrentGameplayActors / (float)MaxGameplayPhysicsActors;
    float ForceLoad = (float)CurrentInteractionForces / (float)MaxInteractionForces;
    
    float AverageLoad = (ActorLoad + ForceLoad) / 2.0f;
    GameplayPerformanceScore = FMath::Clamp(100.0f - (AverageLoad * 100.0f), 0.0f, 100.0f);
    
    // Log gameplay physics status
    UE_LOG(LogTemp, Log, TEXT("GameplayPhysics Performance:"));
    UE_LOG(LogTemp, Log, TEXT("  Gameplay Actors: %d/%d"), CurrentGameplayActors, MaxGameplayPhysicsActors);
    UE_LOG(LogTemp, Log, TEXT("  Interaction Forces: %d/%d"), CurrentInteractionForces, MaxInteractionForces);
    UE_LOG(LogTemp, Log, TEXT("  Performance Score: %.1f"), GameplayPerformanceScore);
    
    // Broadcast performance update
    OnGameplayPhysicsUpdated.Broadcast(GameplayPerformanceScore);
}

void ACore_PhysicsGameplayIntegrator::EnableRagdollForActor(AActor* Actor)
{
    if (!Actor) return;
    
    USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (SkeletalMesh)
    {
        SkeletalMesh->SetSimulatePhysics(true);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        
        ActiveRagdolls.AddUnique(Actor);
        
        UE_LOG(LogTemp, Warning, TEXT("PhysicsGameplayIntegrator: Enabled ragdoll for %s"), *Actor->GetName());
    }
}

void ACore_PhysicsGameplayIntegrator::DisableRagdollForActor(AActor* Actor)
{
    if (!Actor) return;
    
    USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (SkeletalMesh)
    {
        SkeletalMesh->SetSimulatePhysics(false);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        
        ActiveRagdolls.Remove(Actor);
        
        UE_LOG(LogTemp, Warning, TEXT("PhysicsGameplayIntegrator: Disabled ragdoll for %s"), *Actor->GetName());
    }
}

bool ACore_PhysicsGameplayIntegrator::ValidateGameplayIntegration()
{
    // Validate all registered actors
    bool bAllValid = true;
    
    // Check characters
    for (int32 i = RegisteredCharacters.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(RegisteredCharacters[i]))
        {
            RegisteredCharacters.RemoveAt(i);
            bAllValid = false;
        }
    }
    
    // Check dinosaurs
    for (int32 i = RegisteredDinosaurs.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(RegisteredDinosaurs[i]))
        {
            RegisteredDinosaurs.RemoveAt(i);
            bAllValid = false;
        }
    }
    
    // Check interactables
    for (int32 i = RegisteredInteractables.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(RegisteredInteractables[i]))
        {
            RegisteredInteractables.RemoveAt(i);
            bAllValid = false;
        }
    }
    
    return bAllValid;
}

FString ACore_PhysicsGameplayIntegrator::GetIntegrationStatus()
{
    return FString::Printf(TEXT("Physics Gameplay Integration Status:\n")
                          TEXT("Active: %s\n")
                          TEXT("Characters: %d\n")
                          TEXT("Dinosaurs: %d\n")
                          TEXT("Interactables: %d\n")
                          TEXT("Active Ragdolls: %d\n")
                          TEXT("Interaction Forces: %d/%d\n")
                          TEXT("Performance Score: %.1f"),
                          bIsSystemActive ? TEXT("YES") : TEXT("NO"),
                          RegisteredCharacters.Num(),
                          RegisteredDinosaurs.Num(),
                          RegisteredInteractables.Num(),
                          ActiveRagdolls.Num(),
                          CurrentInteractionForces, MaxInteractionForces,
                          GameplayPerformanceScore);
}