#include "TranspersonalAnimationSubsystem.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

void UTranspersonalAnimationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    GlobalAnimationSpeed = 1.0f;
    bDebugAnimationSystem = false;
    
    LoadDefaultProfiles();
    InitializeMotionMatchingSystem();
    InitializeIKSystem();
    
    UE_LOG(LogTemp, Warning, TEXT("TranspersonalAnimationSubsystem initialized"));
}

void UTranspersonalAnimationSubsystem::LoadDefaultProfiles()
{
    // Perfil do protagonista (paleontologista)
    FCharacterAnimationProfile ProtagonistProfile;
    ProtagonistProfile.CharacterName = TEXT("Protagonist");
    ProtagonistProfile.MovementSpeed = 380.0f; // Ligeiramente mais lento - não é atlético
    ProtagonistProfile.TurnRate = 35.0f; // Movimentos mais cautelosos
    ProtagonistProfile.bCanCrouch = true;
    ProtagonistProfile.bCanClimb = true;
    
    AnimationProfiles.Add(TEXT("Protagonist"), ProtagonistProfile);
    
    // Perfil base para dinossauros pequenos (domesticáveis)
    FCharacterAnimationProfile SmallDinoProfile;
    SmallDinoProfile.CharacterName = TEXT("SmallDinosaur");
    SmallDinoProfile.MovementSpeed = 200.0f;
    SmallDinoProfile.TurnRate = 90.0f; // Mais ágeis
    SmallDinoProfile.bCanCrouch = false;
    SmallDinoProfile.bCanClimb = false;
    
    AnimationProfiles.Add(TEXT("SmallDinosaur"), SmallDinoProfile);
    
    // Perfil base para dinossauros grandes (predadores)
    FCharacterAnimationProfile LargeDinoProfile;
    LargeDinoProfile.CharacterName = TEXT("LargeDinosaur");
    LargeDinoProfile.MovementSpeed = 600.0f;
    LargeDinoProfile.TurnRate = 25.0f; // Movimentos mais pesados
    LargeDinoProfile.bCanCrouch = false;
    LargeDinoProfile.bCanClimb = false;
    
    AnimationProfiles.Add(TEXT("LargeDinosaur"), LargeDinoProfile);
}

void UTranspersonalAnimationSubsystem::InitializeMotionMatchingSystem()
{
    // Sistema Motion Matching será configurado via Animation Blueprints
    // Esta função prepara as configurações globais
    UE_LOG(LogTemp, Warning, TEXT("Motion Matching system initialized"));
}

void UTranspersonalAnimationSubsystem::InitializeIKSystem()
{
    // Sistema IK para adaptação ao terreno
    UE_LOG(LogTemp, Warning, TEXT("IK system initialized for terrain adaptation"));
}

FCharacterAnimationProfile UTranspersonalAnimationSubsystem::GetAnimationProfile(const FString& CharacterName)
{
    if (AnimationProfiles.Contains(CharacterName))
    {
        return AnimationProfiles[CharacterName];
    }
    
    // Retorna perfil padrão se não encontrar
    return AnimationProfiles.Contains(TEXT("Protagonist")) ? 
           AnimationProfiles[TEXT("Protagonist")] : 
           FCharacterAnimationProfile();
}

void UTranspersonalAnimationSubsystem::RegisterAnimationProfile(const FString& CharacterName, const FCharacterAnimationProfile& Profile)
{
    AnimationProfiles.Add(CharacterName, Profile);
    UE_LOG(LogTemp, Warning, TEXT("Animation profile registered for: %s"), *CharacterName);
}

ETerrainType UTranspersonalAnimationSubsystem::AnalyzeTerrainAtLocation(const FVector& Location, AActor* Character)
{
    if (!Character || !Character->GetWorld())
    {
        return ETerrainType::Flat;
    }
    
    UWorld* World = Character->GetWorld();
    FVector StartLocation = Location + FVector(0, 0, 100);
    FVector EndLocation = Location - FVector(0, 0, 200);
    
    FHitResult HitResult;
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(Character);
    
    if (World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, CollisionParams))
    {
        FVector Normal = HitResult.Normal;
        float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector)));
        
        if (bDebugAnimationSystem)
        {
            DrawDebugLine(World, StartLocation, EndLocation, FColor::Red, false, 1.0f);
            DrawDebugSphere(World, HitResult.Location, 10.0f, 8, FColor::Green, false, 1.0f);
        }
        
        // Análise do tipo de terreno baseada no ângulo da superfície
        if (SlopeAngle > 45.0f)
        {
            return ETerrainType::Steep;
        }
        else if (SlopeAngle > 25.0f)
        {
            return ETerrainType::Rocky;
        }
        else if (SlopeAngle > 10.0f)
        {
            return ETerrainType::Uneven;
        }
        
        // Análise adicional baseada no material da superfície
        if (HitResult.GetComponent())
        {
            FString ComponentName = HitResult.GetComponent()->GetName().ToLower();
            
            if (ComponentName.Contains(TEXT("water")))
            {
                return ETerrainType::Water;
            }
            else if (ComponentName.Contains(TEXT("mud")))
            {
                return ETerrainType::Muddy;
            }
            else if (ComponentName.Contains(TEXT("sand")))
            {
                return ETerrainType::Sandy;
            }
            else if (ComponentName.Contains(TEXT("grass")))
            {
                return ETerrainType::Grass;
            }
        }
    }
    
    return ETerrainType::Flat;
}

float UTranspersonalAnimationSubsystem::CalculateTerrainAdaptationWeight(ETerrainType TerrainType)
{
    switch (TerrainType)
    {
        case ETerrainType::Flat:
            return 0.0f;
        case ETerrainType::Uneven:
            return 0.3f;
        case ETerrainType::Rocky:
            return 0.6f;
        case ETerrainType::Muddy:
            return 0.4f;
        case ETerrainType::Sandy:
            return 0.2f;
        case ETerrainType::Grass:
            return 0.1f;
        case ETerrainType::Water:
            return 0.8f;
        case ETerrainType::Steep:
            return 1.0f;
        default:
            return 0.0f;
    }
}

void UTranspersonalAnimationSubsystem::UpdateGlobalAnimationSettings(float DeltaTime)
{
    // Atualização global do sistema de animação
    // Pode ser usado para efeitos como fadiga, ferimentos, etc.
}