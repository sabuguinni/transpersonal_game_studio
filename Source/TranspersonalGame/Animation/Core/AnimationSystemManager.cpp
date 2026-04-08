#include "AnimationSystemManager.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AAnimationSystemManager::AAnimationSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.016f; // 60 FPS para responsividade máxima

    // Inicializar estados emocionais base
    EmotionalStateWeights.Add(TEXT("Fear"), 0.0f);
    EmotionalStateWeights.Add(TEXT("Curiosity"), 0.3f);
    EmotionalStateWeights.Add(TEXT("Determination"), 0.5f);
    EmotionalStateWeights.Add(TEXT("Exhaustion"), 0.0f);
    EmotionalStateWeights.Add(TEXT("Alertness"), 0.7f);
}

void AAnimationSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Animation System Manager: Sistema de animação iniciado"));
    
    // Configurar sistema de Motion Matching
    if (PlayerLocomotionDatabase)
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation System: Player Locomotion Database carregada"));
    }
    
    if (DinosaurBehaviorDatabase)
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation System: Dinosaur Behavior Database carregada"));
    }
}

void AAnimationSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Actualizar sistemas de animação em tempo real
    UpdateMotionMatchingQueries(DeltaTime);
    ProcessTerrainAdaptation(DeltaTime);
    UpdateEmotionalStates(DeltaTime);
}

void AAnimationSystemManager::RegisterCharacterForAnimation(ACharacter* Character)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("Animation System: Tentativa de registar personagem nulo"));
        return;
    }

    if (!RegisteredCharacters.Contains(Character))
    {
        RegisteredCharacters.Add(Character);
        
        // Inicializar dados de animação para o personagem
        FCharacterAnimationData NewAnimData;
        NewAnimData.CurrentFearLevel = 0.3f; // Nível base de tensão
        NewAnimData.MovementIntensity = 1.0f;
        NewAnimData.bIsInDanger = false;
        
        AnimationDataCache.Add(Character, NewAnimData);
        
        UE_LOG(LogTemp, Warning, TEXT("Animation System: Personagem %s registado no sistema de animação"), 
               *Character->GetName());
    }
}

void AAnimationSystemManager::UpdateCharacterFearLevel(ACharacter* Character, float NewFearLevel)
{
    if (!Character || !AnimationDataCache.Contains(Character))
    {
        return;
    }

    FCharacterAnimationData& AnimData = AnimationDataCache[Character];
    float PreviousFear = AnimData.CurrentFearLevel;
    
    // Suavizar transições de medo para evitar mudanças bruscas
    AnimData.CurrentFearLevel = FMath::FInterpTo(AnimData.CurrentFearLevel, 
                                                 FMath::Clamp(NewFearLevel, 0.0f, 1.0f), 
                                                 GetWorld()->GetDeltaSeconds(), 2.0f);
    
    // Actualizar intensidade de movimento baseada no medo
    AnimData.MovementIntensity = 1.0f + (AnimData.CurrentFearLevel * 0.5f);
    
    // Broadcast mudança de estado emocional
    OnCharacterEmotionalStateChanged.Broadcast(Character, AnimData.CurrentFearLevel);
    
    // Log apenas mudanças significativas
    if (FMath::Abs(PreviousFear - AnimData.CurrentFearLevel) > 0.1f)
    {
        UE_LOG(LogTemp, Log, TEXT("Animation System: %s fear level: %.2f -> %.2f"), 
               *Character->GetName(), PreviousFear, AnimData.CurrentFearLevel);
    }
}

void AAnimationSystemManager::TriggerEmergencyAnimation(ACharacter* Character, const FString& TriggerType)
{
    if (!Character || !AnimationDataCache.Contains(Character))
    {
        return;
    }

    FCharacterAnimationData& AnimData = AnimationDataCache[Character];
    
    // Marcar como em perigo
    AnimData.bIsInDanger = true;
    AnimData.TimeSinceLastThreat = 0.0f;
    
    // Aumentar drasticamente o nível de medo
    UpdateCharacterFearLevel(Character, 1.0f);
    
    // Adicionar tag emocional específica
    if (!AnimData.ActiveEmotionalTags.Contains(TriggerType))
    {
        AnimData.ActiveEmotionalTags.Add(TriggerType);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Animation System: Emergency animation triggered for %s - Type: %s"), 
           *Character->GetName(), *TriggerType);
}

void AAnimationSystemManager::GenerateUniqueMovementVariation(ACharacter* Character)
{
    if (!Character || !AnimationDataCache.Contains(Character))
    {
        return;
    }

    // Gerar variação procedural baseada em características únicas do personagem
    FCharacterAnimationData& AnimData = AnimationDataCache[Character];
    
    // Criar seed único baseado no nome do personagem
    int32 CharacterSeed = GetTypeHash(Character->GetName());
    FRandomStream RandomStream(CharacterSeed);
    
    // Gerar variações subtis na intensidade de movimento
    float BaseVariation = RandomStream.FRandRange(0.9f, 1.1f);
    AnimData.MovementIntensity *= BaseVariation;
    
    UE_LOG(LogTemp, Log, TEXT("Animation System: Generated unique movement variation for %s (Intensity: %.2f)"), 
           *Character->GetName(), AnimData.MovementIntensity);
}

void AAnimationSystemManager::UpdateMotionMatchingQueries(float DeltaTime)
{
    // Actualizar queries do Motion Matching para todos os personagens registados
    for (ACharacter* Character : RegisteredCharacters)
    {
        if (!Character || !AnimationDataCache.Contains(Character))
        {
            continue;
        }

        const FCharacterAnimationData& AnimData = AnimationDataCache[Character];
        
        // Aqui integramos com o sistema de Motion Matching do UE5
        // Os dados emocionais influenciam a selecção de poses
        
        if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
        {
            if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
            {
                // Passar dados emocionais para o Animation Blueprint
                // Isto será expandido quando tivermos os Animation Blueprints criados
            }
        }
    }
}

void AAnimationSystemManager::ProcessTerrainAdaptation(float DeltaTime)
{
    // Sistema de IK para adaptação ao terreno
    for (ACharacter* Character : RegisteredCharacters)
    {
        if (!Character)
        {
            continue;
        }

        FVector CharacterLocation = Character->GetActorLocation();
        
        // Raycast para detectar terreno irregular
        FHitResult HitResult;
        FVector StartTrace = CharacterLocation + FVector(0, 0, 50);
        FVector EndTrace = CharacterLocation - FVector(0, 0, TerrainAdaptationRadius);
        
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Character);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, 
                                                ECC_WorldStatic, QueryParams))
        {
            // Calcular adaptação IK baseada na inclinação do terreno
            FVector SurfaceNormal = HitResult.Normal;
            float SlopeAngle = FMath::Acos(FVector::DotProduct(SurfaceNormal, FVector::UpVector));
            
            // Debug visual (apenas em desenvolvimento)
            #if WITH_EDITOR
            DrawDebugLine(GetWorld(), StartTrace, HitResult.Location, FColor::Green, false, 0.1f);
            #endif
        }
    }
}

void AAnimationSystemManager::UpdateEmotionalStates(float DeltaTime)
{
    // Actualizar estados emocionais globais
    for (auto& CharacterData : AnimationDataCache)
    {
        FCharacterAnimationData& AnimData = CharacterData.Value;
        
        // Decrementar medo gradualmente se não há ameaças
        if (!AnimData.bIsInDanger)
        {
            AnimData.TimeSinceLastThreat += DeltaTime;
            
            // Após 30 segundos sem ameaças, começar a reduzir o medo
            if (AnimData.TimeSinceLastThreat > 30.0f)
            {
                float FearReduction = DeltaTime * 0.1f; // Redução gradual
                AnimData.CurrentFearLevel = FMath::Max(0.2f, AnimData.CurrentFearLevel - FearReduction);
            }
        }
        
        // Limpar tags emocionais antigas
        if (AnimData.TimeSinceLastThreat > 10.0f)
        {
            AnimData.bIsInDanger = false;
            AnimData.ActiveEmotionalTags.Empty();
        }
    }
}